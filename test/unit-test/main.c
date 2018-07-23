#include"taskSch.h"
#include<stdio.h>
#include<check.h>
#include<stdlib.h>

int init(struct sch_task_pool_t * pool){}
int deinit(struct sch_task_pool_t * pool){}

int perform_task(struct sch_task_package_t *package){

	int fib0 = 1;
	int fib1 = 1;

	for(int i = 0; i < 8000000; i++){
		fib0 = fib1;
		fib1 = fib1 + fib0;
	}
	printf("%d:%d\n", package->index, fib1);
}

START_TEST(create){
	schTaskSch taskSch;
	schTaskSch* sch = &taskSch;

	int i;
	const size_t numPackages = 2048;

	schCreateTaskPool(sch, -1, 0, numPackages);
	ck_assert_int_eq(sch->num, schGetNumCPUCores());

	for(i = 0; i < sch->num; i++){
		ck_assert_int_eq(sch->pool[i].reserved, numPackages);
		ck_assert_int_eq(sch->pool[i].size, 0);
		ck_assert_int_eq(sch->pool[i].flag & SCH_POOL_RUNNING, 0);

		ck_assert_int_eq(sch->pool[i].head, 0);
		ck_assert_int_eq(sch->pool[i].tail, 0);

		ck_assert_int_eq(sch->pool[i].mutex, NULL);
		ck_assert_int_eq(sch->pool[i].thread, NULL);


		ck_assert_int_eq(sch->pool[i].index, i);

		ck_assert_int_eq(sch->pool[i].init, NULL);
		ck_assert_int_eq(sch->pool[i].deinit, NULL);
		ck_assert_int_eq(sch->pool[i].userdata, NULL);



	}

	schSetInitCallBack(sch, init);
	schSetDeInitCallBack(sch, deinit);
	schSetSchUserData(sch, init);

	for(i = 0; i < sch->num; i++){
		ck_assert_ptr_eq(sch->pool[i].init, init);
		ck_assert_ptr_eq(sch->pool[i].deinit, deinit);
		ck_assert_ptr_eq(sch->pool[i].userdata, init);
	}

	/*  */
	ck_assert_int_eq(sch->flag & SCH_FLAG_RUNNING, 0);
	ck_assert_int_eq(sch->flag & SCH_FLAG_INIT, SCH_FLAG_INIT);

	ck_assert(schReleaseTaskSch(sch) == SCH_OK);
}
END_TEST


Suite* schCreateSuite(void){

	/*	Create suite and test cases.	*/
	Suite* suite = suite_create("task-scheduler");
	TCase* testCreate = tcase_create("create");

	/*	Link test cases with functions.	*/
	tcase_add_test(testCreate, create);

	/*	Add test cases to test suite.	*/
	suite_add_tcase(suite, testCreate);

	return suite;
}

void schCreationUnitTest(void) {

	int number_failed;
	Suite *s;
	SRunner *sr;

	/*	Create test suits */
	s = schCreateSuite();

	/*  Create suite runner.    */
	sr = srunner_create(s);
	srunner_run_all(sr, CK_VERBOSE);

	/*	Retrieve number of failures. */
	number_failed = srunner_ntests_failed(sr);

	/*  Display number of total failures and clean up.  */
	printf("number of total failure : %d\n", number_failed);
	printf("\n");
	srunner_free(sr);
}

int main(int argc, const char **argv) {

	/*  Simple unit test.   */
	schCreationUnitTest();

	schTaskSch sch;

	const size_t numPackages = 48;

	schCreateTaskPool(&sch, -1, 0, numPackages);

	schRunTaskSch(&sch);
	long int t = schGetTime();

	schTaskPackage package = {NULL};
	package.callback = perform_task;
	for(int i = 0; i < numPackages; i++)
		schSubmitTask(&sch, &package, NULL);

	schWaitTask(&sch);
	schTerminateTaskSch(&sch);
	t = schGetTime() - t;
	printf("time: %f seconds for %d tasks.\n", (float)t / (float)schTimeResolution(), numPackages);

	return EXIT_SUCCESS;
}

