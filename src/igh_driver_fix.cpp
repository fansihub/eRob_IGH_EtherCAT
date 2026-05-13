/* For CPU_ZERO and CPU_SET macros */
//#define _GNU_SOURCE

/*****************************************************************************/
#include "../../../ethercat_igh/output/include/ecrt.h" 
#include <string.h>
#include <stdio.h>
/* For setting the process's priority (setpriority) */
#include <sys/resource.h>
/* For pid_t and getpid() */
#include <unistd.h>
#include <sys/types.h>
/* For locking the program in RAM (mlockall) to prevent swapping */



#include <sys/mman.h>
/* clock_gettime, struct timespec, etc. */
#include <time.h>
/* Header for handling signals (definition of SIGINT) */
#include <signal.h>
/* For using real-time scheduling policy (FIFO) and sched_setaffinity */
#include <sched.h>
/* For using uint32_t format specifier, PRIu32 */
#include <inttypes.h>

/*****************************************************************************/
/* Comment to disable PDO configuration (i.e. in case the PDO configuration saved in EEPROM is our
   desired configuration.)
*/
#define CONFIG_PDOS

/* Comment to disable distributed clocks. */
#define DC

/* Choose the syncronization method: The reference clock can be either master's, or the reference slave's (slave 0 by default) */
#ifdef DC

/* Slave0's clock is the reference: no drift. Algorithm from rtai_rtdm_dc example. Work in progress. */
//#define SYNC_MASTER_TO_REF
/* Master's clock (CPU) is the reference: lower overhead. */
#define SYNC_REF_TO_MASTER

#endif

#ifdef DC

/* Comment to disable configuring slave's DC specification (shift time & cycle time) */
#define CONFIG_DC

#endif

/*****************************************************************************/

/* One motor revolution increments the encoder by 2^19 -1. */
#define ENCODER_RES 524287
/* The maximum stack size which is guranteed safe to access without faulting. */
#define MAX_SAFE_STACK (8 * 1024)

/* Uncomment to enable performance measurement. */
/* Measure the difference in reference slave's clock timstamp each cycle, and print the result,
   which should be as close to cycleTime as possible. */
/* Note: Only works with DC enabled. */
#define PRINT_STATUS_EVERY_CYCLES FREQUENCY

/* Calculate the time it took to complete the loop. */
/* Keep this disabled during motor tests: printing every 1 ms can break the
   EtherCAT watchdog timing and make the drive drop in and out of enable. */
// #define MEASURE_TIMING

#define SET_CPU_AFFINITY

#define NSEC_PER_SEC (1000000000L)
#define FREQUENCY 1000
/* Period of motion loop, in nanoseconds */
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)
#define MODE_CSV 9
#define TARGET_VELOCITY_CSV 10000
#define MAX_TORQUE_PER_MILLE 1000
#define PRINT_IGH_LATENCY_EVERY_CYCLES FREQUENCY

#ifdef DC

#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)

#endif

#ifdef CONFIG_DC

/* SYNC0 event happens halfway through the cycle */
#define SHIFT0 (PERIOD_NS/2)

#endif

/*****************************************************************************/
/* Note: Anything relying on definition of SYNC_MASTER_TO_REF is essentially copy-pasted from /rtdm_rtai_dc/main.c */

#ifdef SYNC_MASTER_TO_REF

/* First used in system_time_ns() */
static int64_t  system_time_base = 0LL;
/* First used in sync_distributed_clocks() */
static uint64_t dc_time_ns = 0;
static int32_t  prev_dc_diff_ns = 0;
/* First used in update_master_clock() */
static int32_t  dc_diff_ns = 0;
static unsigned int cycle_ns = PERIOD_NS;
static uint8_t  dc_started = 0;
static int64_t  dc_diff_total_ns = 0LL;
static int64_t  dc_delta_total_ns = 0LL;
static int      dc_filter_idx = 0;
static int64_t  dc_adjust_ns;
#define DC_FILTER_CNT          1024
/** Return the sign of a number
 *
 * ie -1 for -ve value, 0 for 0, +1 for +ve value
 *
 * \retval the sign of the value
 */
#define sign(val) \
    ({ typeof (val) _val = (val); \
    ((_val > 0) - (_val < 0)); })

static uint64_t dc_start_time_ns = 0LL;

#endif

ec_master_t* master;

/*****************************************************************************/

#ifdef SYNC_MASTER_TO_REF

/** Get the time in ns for the current cpu, adjusted by system_time_base.
 *
 * \attention Rather than calling rt_get_time_ns() directly, all application
 * time calls should use this method instead.
 *
 * \ret The time in ns.
 */
uint64_t system_time_ns(void)
{
	struct timespec time;
	int64_t time_ns;
	clock_gettime(CLOCK_MONOTONIC, &time);
	time_ns = TIMESPEC2NS(time);

	if (system_time_base > time_nsec)
	{
		printf("%s() error: system_time_base greater than"
		       " system time (system_time_base: %ld, time: %lu\n",
			__func__, system_time_base, time_ns);
		return time_ns;
	}
	else
	{
		return time_ns - system_time_base;
	}
}


/** Synchronise the distributed clocks
 */
void sync_distributed_clocks(void)
{

	uint32_t ref_time = 0;
	uint64_t prev_app_time = dc_time_ns;

	dc_time_ns = system_time_ns();

	// set master time in nano-seconds
	ecrt_master_application_time(master, dc_time_ns);

	// get reference clock time to synchronize master cycle
	ecrt_master_reference_clock_time(master, &ref_time);
	dc_diff_ns = (uint32_t) prev_app_time - ref_time;

	// call to sync slaves to ref slave
	ecrt_master_sync_slave_clocks(master);
}


/** Update the master time based on ref slaves time diff
 *
 * called after the ethercat frame is sent to avoid time jitter in
 * sync_distributed_clocks()
 */
void update_master_clock(void)
{

	// calc drift (via un-normalised time diff)
	int32_t delta = dc_diff_ns - prev_dc_diff_ns;
	//printf("%d\n", (int) delta);
	prev_dc_diff_ns = dc_diff_ns;

	// normalise the time diff
	dc_diff_ns = ((dc_diff_ns + (cycle_ns / 2)) % cycle_ns) - (cycle_ns / 2);

	// only update if primary master
	if (dc_started)
	{

		// add to totals
		dc_diff_total_ns += dc_diff_ns;
		dc_delta_total_ns += delta;
		dc_filter_idx++;

		if (dc_filter_idx >= DC_FILTER_CNT)
		{
			// add rounded delta average
			dc_adjust_ns += ((dc_delta_total_ns + (DC_FILTER_CNT / 2)) / DC_FILTER_CNT);

			// and add adjustment for general diff (to pull in drift)
			dc_adjust_ns += sign(dc_diff_total_ns / DC_FILTER_CNT);

			// limit crazy numbers (0.1% of std cycle time)
			if (dc_adjust_ns < -1000)
			{
				dc_adjust_ns = -1000;
			}
			if (dc_adjust_ns > 1000)
			{
				dc_adjust_ns =  1000;
			}

			// reset
			dc_diff_total_ns = 0LL;
			dc_delta_total_ns = 0LL;
			dc_filter_idx = 0;
		}

		// add cycles adjustment to time base (including a spot adjustment)
		system_time_base += dc_adjust_ns + sign(dc_diff_ns);
	}
	else
	{
		dc_started = (dc_diff_ns != 0);

		if (dc_started)
		{
			// output first diff
			printf("First master diff: %d.\n", dc_diff_ns);

			// record the time of this initial cycle
			dc_start_time_ns = dc_time_ns;
		}
	}
}

#endif

/*****************************************************************************/

void ODwriteU8(ec_master_t* master, uint16_t slavePos, uint16_t index, uint8_t subIndex, uint8_t objectValue)
{
	/* Blocks until a reponse is received */
	uint8_t retVal = ecrt_master_sdo_download(master, slavePos, index, subIndex, &objectValue, sizeof(objectValue), NULL);
	/* retVal != 0: Failure */
	if (retVal)
		printf("OD write unsuccessful\n");
}

void initDrive(ec_master_t* master, uint16_t slavePos)
{
	/* Mode of operation, CSV */
	ODwriteU8(master, slavePos, 0x6060, 0x00, MODE_CSV);
}

/*****************************************************************************/

/* Add two timespec structures (time1 and time2), store the the result in result. */
/* result = time1 + time2 */
inline void timespec_add(struct timespec* result, struct timespec* time1, struct timespec* time2)
{

	if ((time1->tv_nsec + time2->tv_nsec) >= NSEC_PER_SEC)
	{
		result->tv_sec  = time1->tv_sec + time2->tv_sec + 1;
		result->tv_nsec = time1->tv_nsec + time2->tv_nsec - NSEC_PER_SEC;
	}
	else
	{
		result->tv_sec  = time1->tv_sec + time2->tv_sec;
		result->tv_nsec = time1->tv_nsec + time2->tv_nsec;
	}

}

inline int64_t timespec_diff_ns(const struct timespec* start, const struct timespec* end)
{
	return ((int64_t)end->tv_sec - (int64_t)start->tv_sec) * NSEC_PER_SEC +
		((int64_t)end->tv_nsec - (int64_t)start->tv_nsec);
}

inline void update_latency_range(int64_t value, int64_t* min, int64_t* max)
{
	if (value < *min) {
		*min = value;
	}

	if (value > *max) {
		*max = value;
	}
}

#ifdef MEASURE_TIMING
/* Substract two timespec structures (time1 and time2), store the the result in result.
/* result = time1 - time2 */
inline void timespec_sub(struct timespec* result, struct timespec* time1, struct timespec* time2)
{

	if ((time1->tv_nsec - time2->tv_nsec) < 0)
	{
		result->tv_sec  = time1->tv_sec - time2->tv_sec - 1;
		result->tv_nsec = NSEC_PER_SEC - (time1->tv_nsec - time2->tv_nsec);
	}
	else
	{
		result->tv_sec  = time1->tv_sec - time2->tv_sec;
		result->tv_nsec = time1->tv_nsec - time2->tv_nsec;
	}

}
#endif

/*****************************************************************************/

/* We have to pass "master" to ecrt_release_master in signal_handler, but it is not possible
   to define one with more than one argument. Therefore, master should be a global variable.
*/
void signal_handler(int sig)
{
	printf("\nReleasing master...\n");
	ecrt_release_master(master);
	pid_t pid = getpid();
	kill(pid, SIGKILL);
}

/*****************************************************************************/

/* We make sure 8kB (maximum stack size) is allocated and locked by mlockall(MCL_CURRENT | MCL_FUTURE). */
void stack_prefault(void)
{
    unsigned char dummy[MAX_SAFE_STACK];
    memset(dummy, 0, MAX_SAFE_STACK);
}

/*****************************************************************************/
#define EC_NEWTIMEVAL2NANO(TV) \
(((TV).tv_sec - 946684800ULL) * 1000000000ULL + (TV).tv_nsec)
uint32_t interval_=(uint32_t)(1000000000.0 / 1000);

// Add state definitions
#define STATE_FAULT              0x0008
#define STATE_SWITCH_ON_DISABLED 0x0040
#define STATE_READY_TO_SWITCH_ON 0x0021
#define STATE_SWITCHED_ON        0x0023
#define STATE_OPERATION_ENABLED  0x0027

// Add function to get drive state
uint16_t getDriveState(uint16_t statusWord) {
    return statusWord & 0x6f; // Mask to get state bits
}

const char* driveStateName(uint16_t statusWord) {
	if (statusWord & STATE_FAULT) {
		return "Fault";
	}

	if ((statusWord & 0x004f) == STATE_SWITCH_ON_DISABLED) {
		return "Switch on disabled";
	}

	switch (statusWord & 0x006f) {
		case STATE_READY_TO_SWITCH_ON:
			return "Ready to switch on";
		case STATE_SWITCHED_ON:
			return "Switched on";
		case STATE_OPERATION_ENABLED:
			return "Operation enabled";
		default:
			return "Unknown";
	}
}

// Add these control word commands
#define CONTROL_WORD_SHUTDOWN           0x0006
#define CONTROL_WORD_SWITCH_ON         0x0007
#define CONTROL_WORD_ENABLE_OPERATION  0x000F
#define CONTROL_WORD_FAULT_RESET       0x0080

int main(int argc, char **argv)
{

	#ifdef SET_CPU_AFFINITY
	cpu_set_t set;
	/* Clear set, so that it contains no CPUs. */
	CPU_ZERO(&set);
	/* Add CPU (core) 1 to the CPU set. */
	CPU_SET(7, &set);
	#endif

	/* 0 for the first argument means set the affinity of the current process. */
	/* Returns 0 on success. */
	if (sched_setaffinity(0, sizeof(set), &set))
	{
		printf("Setting CPU affinity failed!\n");
		return -1;
	}

	/* SCHED_FIFO tasks are allowed to run until they have completed their work or voluntarily yield. */
	/* Note that even the lowest priority realtime thread will be scheduled ahead of any thread with a non-realtime policy;
	   if only one realtime thread exists, the SCHED_FIFO priority value does not matter.
	*/
	struct sched_param param = {};
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	printf("Using priority %i.\n", param.sched_priority);
	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
	{
		perror("sched_setscheduler failed\n");
	}

	/* Lock the program into RAM to prevent page faults and swapping */
	/* MCL_CURRENT: Lock in all current pages.
	   MCL_FUTURE:  Lock in pages for heap and stack and shared memory.
	*/
	if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
	{
		printf("mlockall failed\n");
		return -1;
	}

	/* Allocate the entire stack, locked by mlockall(MCL_FUTURE). */
	stack_prefault();
	/* Register the signal handler function. */
	signal(SIGINT, signal_handler);

	/* Reserve the first master (0) (/etc/init.d/ethercat start) for this program */
	master = ecrt_request_master(0);
	if (!master)
	{
		printf("Requesting master failed\n");
		return -1;
	}

	initDrive(master, 0);

	uint16_t alias = 0;
	uint16_t position0 = 0;
	uint32_t vendor_id = 0x5a65726f;
	uint32_t product_code = 0x00029252;

	/* Creates and returns a slave configuration object, ec_slave_config_t*, for the given alias and position. */
	/* Returns NULL (0) in case of error and pointer to the configuration struct otherwise */

	ec_slave_config_t* drive0 = ecrt_master_slave_config(master, alias, position0, vendor_id, product_code);

	ec_slave_config_state_t slaveState0;


	/* If the drive0 = NULL or drive1 = NULL */
	if (!drive0)
	{
		printf("Failed to get slave configuration\n");
		return -1;
	}

	#ifdef CONFIG_PDOS
	/***************************************************/
	/* Slave 0's structures, obtained from $ethercat cstruct -p 0 */
	ec_pdo_entry_info_t slave_0_pdo_entries[] =
	{
	{0x607a, 0x00, 32}, /* Target Position */
	{0x60FF, 0x00, 32}, /* Target Velocity */
	{0x6071, 0x00, 16}, /* Target Torque */
	{0x6072, 0x00, 16}, /* Max Torque */
	{0x6040, 0x00, 16}, /* Controlword */
	{0x6060, 0x00, 8},  /* Modes of Operation */
	{0x0000, 0x00, 8},  /* Dummy byte */

	{0x603f, 0x00, 16}, /* Error Code */
	{0x6041, 0x00, 16}, /* Statusword */
	{0x6064, 0x00, 32}, /* Position Actual Value */
	{0x606c, 0x00, 32}, /* Velocity Actual Value */
	{0x6077, 0x00, 16}, /* Torque Actual Value */
	{0x6061, 0x00, 8},  /* Modes of Operation Display */
	{0x0000, 0x00, 8},  /* Dummy byte */
	};

	ec_pdo_info_t slave_0_pdos[] =
	{
	{0x1605, 7, slave_0_pdo_entries + 0}, /* ZeroErr RxPDO: command + mode */
	{0x1a06, 7, slave_0_pdo_entries + 7}, /* ZeroErr TxPDO: status + diagnostics */
	};

	ec_sync_info_t slave_0_syncs[] =
	{
	{0, EC_DIR_OUTPUT, 0, NULL            , EC_WD_DISABLE},
	{1, EC_DIR_INPUT , 0, NULL            , EC_WD_DISABLE},
	{2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_ENABLE},
	{3, EC_DIR_INPUT , 1, slave_0_pdos + 1, EC_WD_DISABLE},
	{0xFF}
	};

	/***************************************************/

	if (ecrt_slave_config_pdos(drive0, EC_END, slave_0_syncs))
	{
		printf("Failed to configure slave 0 PDOs\n");
		return -1;
	}


	#endif

	uint controlword, statusword ,
	target_position,actual_position,
	target_velocity,actual_velocity,
	target_torque,actual_torque,
	max_torque,modes_of_operation,
	error_code,modes_of_operation_display;
	
	ec_pdo_entry_reg_t domain1_regs[] =
	{
	{0, 0, vendor_id, product_code, 0x607a, 0x00, &target_position     },
	{0, 0, vendor_id, product_code, 0x60FF, 0x00, &target_velocity     },
	{0, 0, vendor_id, product_code, 0x6071, 0x00, &target_torque       },
	{0, 0, vendor_id, product_code, 0x6072, 0x00, &max_torque          },
	{0, 0, vendor_id, product_code, 0x6040, 0x00, &controlword         },
	{0, 0, vendor_id, product_code, 0x6060, 0x00, &modes_of_operation  },
	{0, 0, vendor_id, product_code, 0x603f, 0x00, &error_code          },
	{0, 0, vendor_id, product_code, 0x6041, 0x00, &statusword          },
	{0, 0, vendor_id, product_code, 0x6064, 0x00, &actual_position     },
	{0, 0, vendor_id, product_code, 0x606c, 0x00, &actual_velocity     },
	{0, 0, vendor_id, product_code, 0x6077, 0x00, &actual_torque       },
	{0, 0, vendor_id, product_code, 0x6061, 0x00, &modes_of_operation_display},
	{}
	};
	/* Creates a new process data domain. */
	/* For process data exchange, at least one process data domain is needed. */
	ec_domain_t* domain1 = ecrt_master_create_domain(master);

	/* Registers PDOs for a domain. */
	/* Returns 0 on success. */
	printf("Activating master...\n");


	//(master, 1);
	if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs))
	{
		printf("PDO entry registration failed\n");
		return -1;
	}

	#ifdef CONFIG_DC

	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	ecrt_master_application_time(master, EC_NEWTIMEVAL2NANO(t));
	/* Do not enable Sync1 */
	ecrt_slave_config_dc(drive0, 0x0300, PERIOD_NS,0, 0, 0);


	#endif

	#ifdef SYNC_REF_TO_MASTER
	/* Initialize master application time. */
	struct timespec masterInitTime;
	clock_gettime(CLOCK_MONOTONIC, &masterInitTime);
	ecrt_master_application_time(master, TIMESPEC2NS(masterInitTime));
	#endif

	#ifdef SYNC_MASTER_TO_REF
	/* Initialize master application time. */
	dc_start_time_ns = system_time_ns();
	dc_time_ns = dc_start_time_ns;
	ecrt_master_application_time(master, dc_start_time_ns);

	if (ecrt_master_select_reference_clock(master, drive0))
	{
		printf("Selecting slave 0 as reference clock failed!\n");
		return -1;
	}
	#endif

	/* Up to this point, we have only requested the master. See log messages */
	printf("Activating master...\n");

	if (ecrt_master_activate(master))
		return -1;

	uint8_t* domain1_pd;
	/* Returns a pointer to (I think) the first byte of PDO data of the domain */
	if (!(domain1_pd = ecrt_domain_data(domain1)))
		return -1;

	struct timespec wakeupTime;

	#ifdef DC
	struct timespec	time;
	#endif

	struct timespec cycleTime = {0, PERIOD_NS};
	clock_gettime(CLOCK_MONOTONIC, &wakeupTime);

	/* The slaves (drives) enter OP mode after exchanging a few frames. */
	/* We exchange frames with no RPDOs (targetPos) untill all slaves have
	   reached OP state, and then we break out of the loop.
	*/
	while (1)
	{

		timespec_add(&wakeupTime, &wakeupTime, &cycleTime);
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime, NULL);

		ecrt_master_receive(master);

		ecrt_slave_config_state(drive0, &slaveState0);

		if (slaveState0.operational)
		{
			printf("All slaves have reached OP state\n");
			//initDrive(master, 0);
			break;
		}

		ecrt_domain_queue(domain1);

		#ifdef SYNC_REF_TO_MASTER
		/* Syncing reference slave to master:
                   1- The master's (PC) clock is the reference.
		   2- Sync the reference slave's clock to the master's.
		   3- Sync the other slave clocks to the reference slave's.
		*/

		clock_gettime(CLOCK_MONOTONIC, &time);
		ecrt_master_application_time(master, TIMESPEC2NS(time));
		/* Queues the DC reference clock drift compensation datagram for sending.
		   The reference clock will by synchronized to the **application (PC)** time provided
		   by the last call off ecrt_master_application_time().
		*/
		ecrt_master_sync_reference_clock(master);
		/* Queues the DC clock drift compensation datagram for sending.
		   All slave clocks will be synchronized to the reference slave clock.
		*/
		ecrt_master_sync_slave_clocks(master);
		#endif

		#ifdef SYNC_MASTER_TO_REF
		// sync distributed clock just before master_send to set
     	        // most accurate master clock time
                sync_distributed_clocks();
		#endif

		ecrt_master_send(master);

		#ifdef SYNC_MASTER_TO_REF
		// update the master clock
     		// Note: called after ecrt_master_send() to reduce time
                // jitter in the sync_distributed_clocks() call
                update_master_clock();
		#endif

	}

	int32_t actPos0;
	int32_t actVel0;
	#ifdef MEASURE_PERF
	/* The slave time received in the current and the previous cycle */
	uint32_t t_cur, t_prev;
	#endif

	/* Sleep is how long we should sleep each loop to keep the cycle's frequency as close to cycleTime as possible. */
	struct timespec sleepTime;
	#ifdef MEASURE_TIMING
	struct timespec execTime, endTime;
	#endif

	/* Wake up 1 msec after the start of the previous loop. */
	sleepTime = cycleTime;
	/* Update wakeupTime = current time */
	clock_gettime(CLOCK_MONOTONIC, &wakeupTime);

	bool firstStatusLog = true;
	uint32_t statusLogCounter = 0;
	uint16_t lastStatusWord = 0xffff;
	uint16_t lastErrorCode = 0xffff;
	uint16_t lastControlWord = 0xffff;
	int lastModeDisplay = 999;
	uint32_t latencyLogCounter = 0;
	int64_t periodMinNs = INT64_MAX;
	int64_t periodMaxNs = INT64_MIN;
	int64_t wakeMinNs = INT64_MAX;
	int64_t wakeMaxNs = INT64_MIN;
	int64_t execMinNs = INT64_MAX;
	int64_t execMaxNs = INT64_MIN;
	int64_t totalMinNs = INT64_MAX;
	int64_t totalMaxNs = INT64_MIN;
	struct timespec lastWakeTime = wakeupTime;
	struct timespec wakeTime;
	struct timespec sendDoneTime;

	while (1)
	{
		#ifdef MEASURE_TIMING
		clock_gettime(CLOCK_MONOTONIC, &endTime);
		/* wakeupTime is also start time of the loop. */
		/* execTime = endTime - wakeupTime */
		timespec_sub(&execTime, &endTime, &wakeupTime);
		printf("Execution time: %lu ns   ", execTime.tv_nsec);
		#endif

		/* wakeupTime = wakeupTime + sleepTime */
		timespec_add(&wakeupTime, &wakeupTime, &sleepTime);
		/* Sleep to adjust the update frequency */
		/* Note: TIMER_ABSTIME flag is key in ensuring the execution with the desired frequency.
		 *
		 *
		   We don't have to conider the loop's execution time (as long as it doesn't get too close to 1 ms),
		   as the sleep ends cycleTime (=1 msecs) *after the start of the previous loop*.
		*/
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime, NULL);
		clock_gettime(CLOCK_MONOTONIC, &wakeTime);
		/* Fetches received frames from the newtork device and processes the datagrams. */
		ecrt_master_receive(master);
		/* Evaluates the working counters of the received datagrams and outputs statistics,
		   if necessary.
		   This function is NOT essential to the receive/process/send procedure and can be
		   commented out
		*/
		ecrt_domain_process(domain1);

		#ifdef MEASURE_PERF
		ecrt_master_reference_clock_time(master, &t_cur);
		#endif

		/********************************************************************************/

		/* Read PDOs from the datagram */
		actPos0 = EC_READ_S32(domain1_pd + actual_position);
		actVel0 = EC_READ_S32(domain1_pd + actual_velocity);
		uint16_t statusWord = EC_READ_U16(domain1_pd + statusword);
		uint16_t errCode = EC_READ_U16(domain1_pd + error_code);
		int modeDisplay = (int8_t)EC_READ_U8(domain1_pd + modes_of_operation_display);
		uint16_t cw = CONTROL_WORD_SHUTDOWN;

		/* Keep command objects valid before and during enable transitions. */
		EC_WRITE_U8(domain1_pd + modes_of_operation, MODE_CSV);
		EC_WRITE_S32(domain1_pd + target_position, actPos0);
		EC_WRITE_S32(domain1_pd + target_velocity, TARGET_VELOCITY_CSV);
		EC_WRITE_S16(domain1_pd + target_torque, 0);
		EC_WRITE_U16(domain1_pd + max_torque, MAX_TORQUE_PER_MILLE);

		if (statusWord & STATE_FAULT) {
			cw = CONTROL_WORD_FAULT_RESET;
		} else if ((statusWord & 0x004f) == STATE_SWITCH_ON_DISABLED) {
			cw = CONTROL_WORD_SHUTDOWN;
		} else {
			switch(statusWord & 0x006f) {
				case STATE_READY_TO_SWITCH_ON:
					cw = CONTROL_WORD_SWITCH_ON;
					break;

				case STATE_SWITCHED_ON:
				case STATE_OPERATION_ENABLED:
					cw = CONTROL_WORD_ENABLE_OPERATION;
					break;

				default:
					cw = CONTROL_WORD_SHUTDOWN;
					break;
			}
		}

		EC_WRITE_U16(domain1_pd + controlword, cw);

		statusLogCounter++;
		bool statusChanged = firstStatusLog ||
			statusWord != lastStatusWord ||
			errCode != lastErrorCode ||
			modeDisplay != lastModeDisplay ||
			cw != lastControlWord;

		if (statusChanged || statusLogCounter >= PRINT_STATUS_EVERY_CYCLES) {
			printf("status=0x%04x (%s) err=0x%04x opmode=%d cw=0x%04x actPos=%d actVel=%d targetVel=%d\n",
				statusWord, driveStateName(statusWord), errCode, modeDisplay,
				cw, actPos0, actVel0, TARGET_VELOCITY_CSV);

			firstStatusLog = false;
			statusLogCounter = 0;
			lastStatusWord = statusWord;
			lastErrorCode = errCode;
			lastModeDisplay = modeDisplay;
			lastControlWord = cw;
		}

		/********************************************************************************/

		/* Queues all domain datagrams in the master's datagram queue.
		   Call this function to mark the domain's datagrams for exchanging at the
		   next call of ecrt_master_send()
		*/
		ecrt_domain_queue(domain1);

		#ifdef SYNC_REF_TO_MASTER
		/* Distributed clocks */
		clock_gettime(CLOCK_MONOTONIC, &time);
		ecrt_master_application_time(master, TIMESPEC2NS(time));
		ecrt_master_sync_reference_clock(master);
		ecrt_master_sync_slave_clocks(master);
		#endif

		#ifdef SYNC_MASTER_TO_REF
		// sync distributed clock just before master_send to set
     	        // most accurate master clock time
                sync_distributed_clocks();
		#endif

		/* Sends all datagrams in the queue.
		   This method takes all datagrams that have been queued for transmission,
		   puts them into frames, and passes them to the Ethernet device for sending.
		*/
		ecrt_master_send(master);
		clock_gettime(CLOCK_MONOTONIC, &sendDoneTime);

		int64_t periodNs = timespec_diff_ns(&lastWakeTime, &wakeTime);
		int64_t wakeNs = timespec_diff_ns(&wakeupTime, &wakeTime);
		int64_t execNs = timespec_diff_ns(&wakeTime, &sendDoneTime);
		int64_t totalNs = timespec_diff_ns(&wakeupTime, &sendDoneTime);
		lastWakeTime = wakeTime;

		update_latency_range(periodNs, &periodMinNs, &periodMaxNs);
		update_latency_range(wakeNs, &wakeMinNs, &wakeMaxNs);
		update_latency_range(execNs, &execMinNs, &execMaxNs);
		update_latency_range(totalNs, &totalMinNs, &totalMaxNs);

		latencyLogCounter++;
		if (latencyLogCounter >= PRINT_IGH_LATENCY_EVERY_CYCLES) {
			printf("igh_latency period=%" PRId64 "..%" PRId64 " ns (%+.1f..%+.1f us) wake=%" PRId64 "..%" PRId64 " ns exec=%" PRId64 "..%" PRId64 " ns total=%" PRId64 "..%" PRId64 " ns\n",
				periodMinNs, periodMaxNs,
				(periodMinNs - PERIOD_NS) / 1000.0,
				(periodMaxNs - PERIOD_NS) / 1000.0,
				wakeMinNs, wakeMaxNs,
				execMinNs, execMaxNs,
				totalMinNs, totalMaxNs);

			latencyLogCounter = 0;
			periodMinNs = INT64_MAX;
			periodMaxNs = INT64_MIN;
			wakeMinNs = INT64_MAX;
			wakeMaxNs = INT64_MIN;
			execMinNs = INT64_MAX;
			execMaxNs = INT64_MIN;
			totalMinNs = INT64_MAX;
			totalMaxNs = INT64_MIN;
		}

		#ifdef SYNC_MASTER_TO_REF
		// update the master clock
     		// Note: called after ecrt_master_send() to reduce time
                // jitter in the sync_distributed_clocks() call
                update_master_clock();
		#endif

		#ifdef MEASURE_PERF
		printf("\tTimestamp diff: %" PRIu32 " ns\n", t_cur - t_prev);
		t_prev = t_cur;
		#endif

	}

	return 0;
}
