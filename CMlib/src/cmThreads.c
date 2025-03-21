/******************************************************************************

GHAAS Command Line Library V1.0
Global Hydrological Archive and Analysis System
Copyright 1994-2024, UNH - CCNY

cmThreads.c

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <cm.h>

size_t CMthreadProcessorNum () {
	char *procEnv;
	int procNum;

	if (((procEnv = getenv ("GHAASprocessorNum")) != (char *) NULL) && (sscanf (procEnv,"%d",&procNum)))
		return (procNum);
	return (0);
}

CMthreadJob_p CMthreadJobCreate (size_t taskNum, CMthreadUserExecFunc execFunc, void *commonData) {
	size_t taskId;
	CMthreadJob_p job;

	if ((job = (CMthreadJob_p) malloc (sizeof (CMthreadJob_t))) == (CMthreadJob_p) NULL) {
		CMmsgPrint (CMmsgSysError, "Memory allocation error in %s:%d",__FILE__,__LINE__);
		return ((CMthreadJob_p) NULL);
	}
	if ((job->Tasks = (CMthreadTask_p) calloc (taskNum,sizeof (CMthreadTask_t))) == (CMthreadTask_p) NULL) {
		CMmsgPrint (CMmsgSysError, "Memory allocation error in %s:%d",__FILE__,__LINE__);
		free (job);
		return ((CMthreadJob_p) NULL);
	}
	if ((job->SortedTasks = (CMthreadTask_p *) calloc (taskNum,sizeof (CMthreadTask_p))) == (CMthreadTask_p *) NULL) {
		CMmsgPrint (CMmsgSysError, "Memory allocation error in %s:%d",__FILE__,__LINE__);
		free (job->Tasks);
		free (job);
		return ((CMthreadJob_p) NULL);
	}
	if ((job->Groups      = (CMthreadTaskGroup_p) calloc (1,sizeof (CMthreadTaskGroup_t))) == (CMthreadTaskGroup_p)  NULL) {
		CMmsgPrint (CMmsgSysError, "Memory allocation error in %s:%d",__FILE__,__LINE__);
		free (job->SortedTasks);
		free (job->Tasks);
		free (job);
		return ((CMthreadJob_p) NULL);
	}
    job->Groups [0].Start = 0;
    job->Groups [0].End   = taskNum;

	job->GroupNum  = 1;
	job->Completed = 0;
	job->Sorted    = true;
	job->TaskNum   = taskNum;
	for (taskId = 0;taskId < job->TaskNum; ++taskId) {
		job->SortedTasks [taskId]        = job->Tasks + taskId;
		job->Tasks [taskId].Id           = taskId;
		job->Tasks [taskId].Dependents   = (CMthreadTask_p *) NULL;
		job->Tasks [taskId].NDependents  = 0;
		job->Tasks [taskId].Travel       = 0;
		job->Tasks [taskId].TravelSet    = false;
	}
	job->UserFunc = execFunc;
	job->CommonData = (void *) commonData;
	return (job);
}

CMreturn CMthreadJobTaskDependent (CMthreadJob_p job, size_t taskId, size_t *dependents, int dlinknum) {
    int dlink;
	if (taskId    > job->TaskNum) {
		CMmsgPrint (CMmsgAppError,"Invalid task in %s%d",__FILE__,__LINE__);
		return (CMfailed);
	}
    for (dlink = 0; dlink < dlinknum; dlink++)
        if (dependents[dlink] > job->TaskNum) {
            CMmsgPrint (CMmsgAppError,"Invalid dependence in %s%d",__FILE__,__LINE__);
            return (CMfailed);
        }
	if (taskId == *dependents) return (CMsucceeded);
	job->Tasks [taskId].Dependents = (CMthreadTask_p *) malloc (dlinknum * sizeof(CMthreadTask_p));
    if (job->Tasks [taskId].Dependents == (CMthreadTask_p *) NULL) {
		CMmsgPrint (CMmsgSysError, "Memory allocation error in %s:%d",__FILE__,__LINE__);
		return (CMfailed);
    }
    for (dlink = 0; dlink < dlinknum; dlink++)
        job->Tasks [taskId].Dependents[dlink] = job->Tasks + dependents[dlink]; // array of pointers to tasks
    job->Tasks [taskId].NDependents = dlinknum;
    job->Sorted = false;
	return (CMsucceeded);
}

static int _CMthreadJobTaskCompare (const void *lPtr,const void *rPtr) {
	int ret;
	CMthreadTask_p lTaskInit = *((CMthreadTask_p *) lPtr);
	CMthreadTask_p rTaskInit = *((CMthreadTask_p *) rPtr);
	CMthreadTask_p lTask = lTaskInit;
	CMthreadTask_p rTask = rTaskInit;

	if ((ret = (int) rTask->Travel - (int) lTask->Travel) != 0) return (ret);

    return 0;
}

static size_t _CMtravel_dist(CMthreadTask_t *task) { // RECURSIVE
    int depi;
    size_t travel, maxtravel = 0;
    if (task->TravelSet) return task->Travel;
    if (task->Dependents != (CMthreadTask_p *) NULL) {
        for (depi = 0; depi < task->NDependents; depi++) {
            travel = _CMtravel_dist(task->Dependents[depi]) + 1;
            if (travel > maxtravel) maxtravel = travel;
        }
    }
    task->Travel = maxtravel;
    task->TravelSet = true;
    return maxtravel;
}

static CMreturn _CMthreadJobTaskSort (CMthreadJob_p job) {
	size_t taskId, start;
	size_t travel;

	for (taskId = 0;taskId < job->TaskNum; ++taskId) {
        if (job->Tasks[taskId].TravelSet) continue;
        job->Tasks[taskId].Travel = _CMtravel_dist(job->Tasks + taskId);
	}

    qsort (job->SortedTasks,job->TaskNum,sizeof (CMthreadTask_p),_CMthreadJobTaskCompare);
	job->GroupNum = job->SortedTasks [0]->Travel + 1;
	if ((job->Groups = (CMthreadTaskGroup_p) realloc (job->Groups, job->GroupNum * sizeof (CMthreadTaskGroup_t))) == (CMthreadTaskGroup_p) NULL) {
		CMmsgPrint (CMmsgSysError,"Memory allocation error in: %s:%d",__FILE__,__LINE__);
		return (CMfailed);
	}
	travel = job->SortedTasks [0]->Travel;
	start  = 0;
	for (taskId = 0;taskId < job->TaskNum; ++taskId) {
		if (travel != job->SortedTasks [taskId]->Travel) {
			job->Groups [job->GroupNum  - travel - 1].Start = (size_t) start;
			job->Groups [job->GroupNum  - travel - 1].End   = (size_t) taskId;
			travel = job->SortedTasks [taskId]->Travel;
			start = taskId;
		}
	}
    job->Groups [job->GroupNum  - travel - 1].Start = (size_t) start;
    job->Groups [job->GroupNum  - travel - 1].End   = (size_t) taskId;
    job->Sorted = true;
	return (CMsucceeded);
}

void CMthreadJobDestroy (CMthreadJob_p job) {
	size_t group;

	if (job->Groups != (CMthreadTaskGroup_p) NULL) free (job->Groups);
	free (job->Tasks);
	free (job);
}

static void *_CMthreadWork (void *dataPtr) {
	CMthreadData_p data = (CMthreadData_p) dataPtr;
	size_t taskId, start, end, chunkSize, threadNum, workerNum, num, completed;
	CMthreadTeam_p team = (CMthreadTeam_p) data->TeamPtr;
	CMthreadJob_p  job;
    struct timespec req, rem;
    long long startTime;
    struct timeval  tValue;
    struct timezone tZone;

    pthread_mutex_lock (&(team->SMutex));
    do {
        pthread_cond_wait (&(team->SCond), &(team->SMutex));
        job = team->JobPtr;
        if (job != (CMthreadJob_p) NULL) {
            pthread_mutex_unlock(&(team->SMutex));
            gettimeofday(&tValue, &tZone);
            startTime = tValue.tv_sec * 1000000 + tValue.tv_usec;

            start = job->Groups[job->GroupID].Start;
            end   = job->Groups[job->GroupID].End;
            threadNum = end - start < team->ThreadNum ? end - start : team->ThreadNum;
            chunkSize = (size_t) ceil ((double) (end - start) / (double) threadNum);
            start = start + data->Id * chunkSize;
            end   = start + chunkSize < end ? start + chunkSize : end;
            for (taskId = start; taskId < end; ++taskId) {
                job->UserFunc(data->Id, job->SortedTasks[taskId]->Id, job->CommonData);
            }
            gettimeofday(&tValue, &tZone);
            data->Time += (tValue.tv_sec * 1000000 + tValue.tv_usec - startTime);
            pthread_mutex_lock (&(team->SMutex));
            job->Completed++;
            if (job->Completed == team->ThreadNum) {
                pthread_mutex_lock   (&(team->MMutex));
                pthread_cond_signal  (&(team->MCond));
                pthread_mutex_unlock (&(team->MMutex));
            }
        }
    } while (job != (CMthreadJob_p) NULL);
    pthread_mutex_unlock (&(team->SMutex));
    pthread_exit ((void *) NULL);
}

CMreturn CMthreadJobExecute (CMthreadTeam_p team, CMthreadJob_p job) {
    size_t taskId, groupID, start, end;
    long long startTime, localStart;
    struct timeval  tValue;
    struct timezone tZone;

    gettimeofday(&tValue, &tZone);
    startTime = tValue.tv_sec * 1000000 + tValue.tv_usec;

    if (job->Sorted == false) {
        _CMthreadJobTaskSort(job);
        gettimeofday(&tValue, &tZone);
        team->ExecTime += (tValue.tv_sec * 1000000 + tValue.tv_usec - startTime);
    }
    gettimeofday(&tValue, &tZone);
    startTime = tValue.tv_sec * 1000000 + tValue.tv_usec;

    if (team->ThreadNum <= 1) {
        gettimeofday(&tValue, &tZone);
        localStart = tValue.tv_sec * 1000000 + tValue.tv_usec;
        for (groupID = 0; groupID < job->GroupNum; groupID++){
            start = job->Groups[groupID].Start;
            end   = job->Groups[groupID].End;
            for (taskId = start; taskId < end; ++taskId) job->UserFunc(0, job->SortedTasks[taskId]->Id, job->CommonData);
        }
        gettimeofday(&tValue, &tZone);
        team->Time += (tValue.tv_sec * 1000000 + tValue.tv_usec - localStart);
    }
    else {
        for (groupID = 0; groupID < job->GroupNum; groupID++) {
            start = job->Groups[groupID].Start;
            end   = job->Groups[groupID].End;
            if (end - start < team->ThreadNum) {
                for (taskId = start; taskId < end; ++taskId) job->UserFunc(0, job->SortedTasks[taskId]->Id, job->CommonData);
            }
            else {
                pthread_mutex_lock     (&(team->SMutex));
                job->Completed = 0;
                job->GroupID = groupID;
                team->JobPtr = (void *) job;
                pthread_cond_broadcast (&(team->SCond));
                pthread_mutex_unlock   (&(team->SMutex));
                pthread_cond_wait (&(team->MCond), &(team->MMutex));
            }
        }
    }
    gettimeofday(&tValue, &tZone);
    team->ExecTime += (tValue.tv_sec * 1000000 + tValue.tv_usec - startTime);
	return (CMsucceeded);
}

CMthreadTeam_p CMthreadTeamCreate (size_t threadNum) {
    int ret;
	size_t threadId;
    pthread_attr_t thread_attr;
    struct timeval  tValue;
    struct timezone tZone;
    CMthreadTeam_p team;

    if ((team = (CMthreadTeam_p) malloc (sizeof (CMthreadTeam_t))) == (CMthreadTeam_p) NULL) {
        CMmsgPrint (CMmsgSysError,"Memory Allocation error in %s:%d",__FILE__,__LINE__);
        return ((CMthreadTeam_p) NULL);
    }
    gettimeofday(&tValue, &tZone);
    team->TotTime = tValue.tv_sec * 1000000 + tValue.tv_usec;
	team->ThreadNum      = threadNum;
	team->JobPtr         = (void *) NULL;
    team->ExecTime       = 0;
    team->ThreadTime     = 0;
    team->Threads        = (CMthreadData_p) NULL;
    team->Time           = 0;

    if (team->ThreadNum > 1) {
        if ((team->Threads = (CMthreadData_p) calloc (threadNum, sizeof(CMthreadData_t))) == (CMthreadData_p) NULL) {
            CMmsgPrint (CMmsgSysError,"Memory Allocation error in %s:%d",__FILE__,__LINE__);
            return ((CMthreadTeam_p) NULL);
        }
        pthread_attr_init (&thread_attr);
        pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setscope       (&thread_attr, PTHREAD_SCOPE_SYSTEM);

        pthread_mutex_init (&(team->MMutex), NULL);
        pthread_cond_init  (&(team->MCond),  NULL);
        pthread_mutex_init (&(team->SMutex), NULL);
        pthread_cond_init  (&(team->SCond),  NULL);
        for (threadId = 0; threadId < team->ThreadNum; ++threadId) {
            team->Threads[threadId].Id      = threadId;
            team->Threads[threadId].TeamPtr = (void *) team;
            team->Threads[threadId].Time    = 0;
            if ((ret = pthread_create(&(team->Threads[threadId].Thread), &thread_attr, _CMthreadWork,
                                      (void *) (team->Threads + threadId))) != 0) {
                CMmsgPrint(CMmsgSysError, "Thread creation returned with error [%d] in %s:%d", ret, __FILE__, __LINE__);
                free(team->Threads);
                free(team);
                return ((CMthreadTeam_p) NULL);
            }
        }
        for (threadId = 0; threadId < team->ThreadNum; ++threadId)
            while (pthread_kill(team->Threads[threadId].Thread,0) != 0); // Signal 0 just checks if the thread is up without sending any signal 
        pthread_attr_destroy(&thread_attr);
        pthread_mutex_lock (&(team->MMutex));
    }
	return (team);
}

void CMthreadTeamPrintReport (CMmsgType msgType, CMthreadTeam_p team) {
    struct timeval  tValue;
    struct timezone tZone;

    gettimeofday(&tValue, &tZone);
    team->TotTime = tValue.tv_sec * 1000000 + tValue.tv_usec - team->TotTime;
    CMmsgPrint (msgType,"Total Time: %.1f, Execute Time: %.1f, Average Thread Time %.1f, Master Time %.1f",
                (float) team->TotTime    / 1000000.0,
                (float) team->ExecTime   / 1000000.0,
                (float) team->ThreadTime / (team->ThreadNum > 0 ? (float) team->ThreadNum : 1.0) / 1000000.0,
                (float) team->Time       / 1000000.0);
}

void CMthreadTeamDelete (CMthreadTeam_p team) {
    size_t threadId;
    void *status;

    if (team->ThreadNum > 1) {
        team->JobPtr = (CMthreadJob_p) NULL;
        pthread_mutex_lock     (&(team->SMutex));
        pthread_cond_broadcast (&(team->SCond));
        pthread_mutex_unlock   (&(team->SMutex));
        for (threadId = 0; threadId < team->ThreadNum; ++threadId) {
            pthread_join(team->Threads[threadId].Thread, &status);
            team->ThreadTime += team->Threads[threadId].Time;
        }
        pthread_mutex_unlock (&(team->MMutex));
        pthread_mutex_destroy(&(team->MMutex));
        pthread_mutex_destroy(&(team->SMutex));
        pthread_cond_destroy (&(team->SCond));
        free(team->Threads);
    }
    free (team);
}
