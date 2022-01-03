#include"taskSch.h"
#include"internal/internal_structures.h"
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

START_TEST(create) {
	schTaskSch *sch;

	int coreSet[] = {
			schGetNumCPUCores(),
			schGetNumCPUCores() / 2,
			schGetNumCPUCores() / 4
	};

	int i, jcores;
	const size_t numPackages = 2048;
	const int nCores = schGetNumCPUCores();

	schAllocateTaskPool(&sch);
	for (jcores = -1; jcores <= nCores; jcores++) {

		int status = schCreateTaskPool(sch, jcores, 0, numPackages);
		ck_assert_int_eq(status, SCH_OK);
		ck_assert_ptr_ne(sch->pool, NULL);

		for (i = 0; i < sch->num; i++) {
			ck_assert_int_eq(sch->pool[i].reserved, numPackages);
			ck_assert_int_eq(sch->pool[i].size, 0);

			ck_assert_int_eq(sch->pool[i].head, 0);
			ck_assert_int_eq(sch->pool[i].tail, 0);

			ck_assert_int_eq(sch->pool[i].flag & SCH_POOL_RUNNING, 0);
			ck_assert_int_eq(sch->pool[i].flag & SCH_FLAG_RUNNING & SCH_FLAG_INIT, 0);

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

		/*  Check each pool.    */
		for (i = 0; i < sch->num; i++) {
			ck_assert_ptr_eq(sch->pool[i].init, init);
			ck_assert_ptr_eq(sch->pool[i].deinit, deinit);
			ck_assert_ptr_eq(sch->pool[i].userdata, init);
		}

		/*  */
		ck_assert_int_eq(sch->flag & (SCH_FLAG_RUNNING | SCH_FLAG_NO_AFM), 0);
		ck_assert_int_eq(sch->flag, SCH_FLAG_INIT);
		ck_assert_ptr_ne(sch->spinlock, NULL);
		ck_assert_ptr_ne(sch->mutex, NULL);
		ck_assert_ptr_ne(sch->conditional, NULL);
		ck_assert_ptr_ne(sch->barrier, NULL);
		ck_assert_ptr_ne(sch->set, NULL);
		ck_assert_ptr_ne(sch->dheap, NULL);

		ck_assert_int_eq(schTerminateTaskSch(sch), SCH_ERROR_INVALID_STATE);

		/*  Test states.    */
		ck_assert_int_eq(sch->flag & SCH_FLAG_RUNNING, 0);

		/*  */
		ck_assert_int_eq(schReleaseTaskSch(sch), SCH_OK);
	}
	free(sch);
}
END_TEST

START_TEST(submit) {
	schTaskSch taskSch;
	schTaskSch* sch = &taskSch;

	const size_t numPackages = 28;

	int status = schCreateTaskPool(sch, -1, 0, numPackages);
	ck_assert_int_eq(status, SCH_OK);
	ck_assert_int_eq(sch->num, schGetNumCPUCores());

	ck_assert_int_eq(schRunTaskSch(sch), SCH_OK);

	schTaskPackage package = {NULL};
	package.callback = perform_task;
	for(int i = 0; i < numPackages; i++){
		int status = schSubmitTask(sch, &package, NULL);
		ck_assert_int_eq(status, SCH_OK);
	}

	ck_assert_int_eq(schWaitTask(sch), SCH_OK);

	/*  Perform second task submitting. */
	package.callback = perform_task;
	for(int i = 0; i < numPackages; i++){
		int status = schSubmitTask(sch, &package, NULL);
		ck_assert_int_eq(status, SCH_OK);
	}

	/*  Terminate first then release.   */
	ck_assert_int_eq(schTerminateTaskSch(sch), SCH_OK);

	ck_assert_int_ne(sch->flag & SCH_FLAG_RUNNING, 0);

	ck_assert_int_eq(schReleaseTaskSch(sch), SCH_OK);
}
END_TEST

START_TEST(MissUse) {
	schTaskSch taskSch;
	schTaskSch* sch = &taskSch;

	int i, status;
	const size_t numPackages = -2;

	status = schCreateTaskPool(sch, -1, 0, numPackages);
	ck_assert_int_eq(status, SCH_ERROR_INVALID_ARG);
	status = schCreateTaskPool(sch, -1, 0, numPackages);
	ck_assert_int_eq(status, SCH_ERROR_INVALID_ARG);
}
END_TEST

START_TEST(PrimitiveMissUse)
{
	schBarrier* barrier;
	ck_assert_int_eq(schCreateBarrier(&barrier), SCH_OK);
	ck_assert_int_eq(schInitBarrier(barrier, 0), SCH_ERROR_INVALID_ARG);
	ck_assert_int_eq(schDeleteBarrier(barrier), SCH_OK);
}
END_TEST

START_TEST(wait) {
	schTaskSch taskSch;
	schTaskSch* sch = &taskSch;

	const size_t numPackages = 28;

	int status = schCreateTaskPool(sch, -1, 0, numPackages);
	ck_assert_int_eq(status, SCH_OK);
	ck_assert_int_eq(sch->num, schGetNumCPUCores());

	ck_assert_int_eq(schRunTaskSch(sch), SCH_OK);

	ck_assert_int_eq(schWaitTask(sch), SCH_OK);
	ck_assert_int_eq(schTerminateTaskSch(sch), SCH_OK);
	ck_assert_int_eq(schReleaseTaskSch(sch), SCH_OK);
}
END_TEST

START_TEST(Primitive) {
	schBarrier* barrier;
	schMutex * mutex;
	schSpinLock* spinLock;
	schSemaphore* semaphore;
	schConditional* conditional;

	/*  Assert Testing. */
	ck_assert_int_eq(schCreateBarrier(&barrier), SCH_OK);

	ck_assert_int_eq(schInitBarrier(barrier, 1), SCH_OK);
	ck_assert_int_eq(schWaitBarrier(barrier), SCH_OK);
	ck_assert_int_eq(schDeleteBarrier(barrier), SCH_OK);

	ck_assert_int_eq(schCreateConditional(&conditional), SCH_OK);
	ck_assert_int_eq(schConditionalSignal(conditional), SCH_OK);

	/*  */
	ck_assert_int_eq(schCreateMutex(&mutex), SCH_OK);
	ck_assert_int_eq(schMutexLock(mutex), SCH_OK);
	ck_assert_int_eq(schMutexTryLock(mutex, 0), SCH_OK);

}
END_TEST

START_TEST(errorCodeMsg) {

	/*  */
	ck_assert_ptr_eq(schErrorMsg(SCH_OK + 1), NULL);
	ck_assert_ptr_eq(schErrorMsg(SCH_ERROR_PERMISSION_DENIED - 1), NULL);

	for (int i = SCH_ERROR_PERMISSION_DENIED; i < SCH_ERROR_UNKNOWN; i++) {
		ck_assert_ptr_ne(schErrorMsg(i), NULL);
	}
}
END_TEST

Suite* schCreateSuite(void){

	/*	Create suite and test cases.	*/
	Suite* suite = suite_create("task-scheduler");
	TCase* testCreate = tcase_create("create_no_error");
	TCase *testRelease = tcase_create("release_no_error");
	TCase *testSubmit = tcase_create("submit_on_stack");
	TCase* testWait = tcase_create("Wait");
	TCase* testMissUse = tcase_create("MissUse");
	TCase* testPrimitiveMissUse = tcase_create("PrimitiveMissUse");
	TCase* testPrimitive = tcase_create("Primitive");
	TCase* testErrorMsgCodes = tcase_create("error-code-message");

	/*	Link test cases with functions.	*/
	tcase_add_test(testCreate, create);
	tcase_add_test(testSubmit, submit);
	tcase_add_test(testWait, wait);
	tcase_add_test(testMissUse, MissUse);
	tcase_add_test(testPrimitiveMissUse, PrimitiveMissUse);
	tcase_add_test(testPrimitive, Primitive);
	tcase_add_test(testErrorMsgCodes, errorCodeMsg);

	tcase_set_timeout(testCreate, 5);
	tcase_set_timeout(testRelease, 5);
	tcase_set_timeout(testSubmit, 5);
	tcase_set_timeout(testWait, 5);
	tcase_set_timeout(testMissUse, 5);
	tcase_set_timeout(testPrimitiveMissUse, 5);
	tcase_set_timeout(testPrimitive, 5);
	tcase_set_timeout(testErrorMsgCodes, 5);

	/*	Add test cases to test suite.	*/
	suite_add_tcase(suite, testCreate);
	suite_add_tcase(suite, testSubmit);
	suite_add_tcase(suite, testWait);
	suite_add_tcase(suite, testMissUse);
	suite_add_tcase(suite, testPrimitiveMissUse);
	suite_add_tcase(suite, testPrimitive);
	suite_add_tcase(suite, testErrorMsgCodes);

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
	//schCreationUnitTest();

	//TODO relocate.
	schTaskSch* sch;

	const size_t numPackages = 250;

	if(schAllocateTaskPool(&sch) != SCH_OK)
		return EXIT_FAILURE;

	if(schCreateTaskPool(sch, -1, 0, numPackages) != SCH_OK)
		return EXIT_FAILURE;

	if(schRunTaskSch(sch) != SCH_OK)
		return EXIT_FAILURE;
	long int t = schGetTime();

	schTaskPackage package = {NULL};
	package.callback = perform_task;
	for(int i = 0; i < numPackages; i++)
		schSubmitTask(sch, &package, NULL);

	schWaitTask(sch);

	for(int i = 0; i < numPackages; i++)
		schSubmitTask(sch, &package, NULL);

	if (schReleaseTaskSch(sch) != SCH_OK)
		return EXIT_FAILURE;
	t = schGetTime() - t;
	printf("time: %f seconds for %d tasks.\n", (float)t / (float)schTimeResolution(), numPackages);

	return EXIT_SUCCESS;
}

