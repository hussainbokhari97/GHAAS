#include <cm.h>

static size_t _Iteration = 1000000;

static void _UserFunc(size_t threadId, size_t taskId, void *commonPtr) {
    size_t count;
    for (count = 0; count < _Iteration; count++);
}

int main(int argv, char *argc[]) {
    int ret, threadNum = 1;
    size_t loopNum = 2, timeLoop, taskNum = 10000, taskId;
    size_t _taskId, _taskNum;
    CMthreadTeam_p team = (CMthreadTeam_p) NULL;
    CMthreadJob_p job;

    if ((argv > 1) && (sscanf(argc[1], "%d", &ret) == 1)) threadNum = (size_t) ret > 0 ? ret : 1;
    if ((argv > 2) && (sscanf(argc[2], "%d", &ret) == 1)) taskNum = (size_t) ret;
    if ((argv > 3) && (sscanf(argc[3], "%d", &ret) == 1)) _Iteration = (size_t) ret;
    if ((argv > 4) && (sscanf(argc[4], "%d", &ret) == 1)) loopNum = (size_t) ret;
    printf("%d %d %d %d\n", (int) threadNum, (int) taskNum, (int) _Iteration, (int) loopNum);

    if ((team = CMthreadTeamCreate (threadNum)) == (CMthreadTeam_p) NULL) {
        CMmsgPrint (CMmsgUsrError,"Team initialization error %s, %d",__FILE__,__LINE__);
        return (CMfailed);
    }
    if ((job = CMthreadJobCreate(taskNum, _UserFunc, (void *) NULL)) == (CMthreadJob_p) NULL) {
        CMmsgPrint(CMmsgAppError, "Job creation error in %s:%d", __FILE__, __LINE__);
        CMthreadTeamDelete (team);
        return (CMfailed);
    }

    for (timeLoop = 0; timeLoop < loopNum; ++timeLoop) {
        printf("Time %d\n", (int) timeLoop);
        CMthreadJobExecute(team, job);
    }
    CMthreadJobDestroy(job);
    CMthreadTeamPrintReport (CMmsgInfo, team);
    CMthreadTeamDelete(team);
    return (0);
}
