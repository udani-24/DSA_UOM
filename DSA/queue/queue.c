typedef struct WaitNode {
    int             waitId;
    int             customerId;
    char            model[50];
    int             startDate;
    int             endDate;
    int             priority;
    struct WaitNode *next;
} WaitNode;


void enqueueWaitlist(int customerId, const char *model,
                     int startDate, int endDate, int priority) {
    WaitNode *n = malloc(sizeof(WaitNode));
    if (!n) { printf("Memory error.\n"); exit(1); }
    n->waitId=nextWaitId++;
    n->customerId=customerId;
    strcpy(n->model, model);
    n->startDate=startDate;
    n->endDate=endDate;
    n->priority=priority;
    n->next=NULL;

    if (!waitHead ||
        priority < waitHead->priority ||
        (priority==waitHead->priority && startDate < waitHead->startDate)) {
        n->next=waitHead;
        waitHead=n;
        return;
    }

    WaitNode *cur=waitHead;
    while (cur->next &&
          (cur->next->priority < priority ||
          (cur->next->priority==priority && cur->next->startDate<=startDate)))
        cur=cur->next;

    n->next=cur->next;
    cur->next=n;
}

void displayWaitlist() {
    WaitNode *t = waitHead;
    if (!t) { printf("Priority waitlist is empty.\n"); return; }
    printf("\n========== PRIORITY WAITLIST ==========\n");
    printf("%-6s %-8s %-15s %-12s %-12s %-8s\n",
           "WaitID","CustID","Model","Start","End","Priority");
    printf("---------------------------------------------------------------\n");
    while (t) {
        Date s, e; char ss[12], es[12];
        s.year=t->startDate/10000; s.month=(t->startDate/100)%100; s.day=t->startDate%100;
        e.year=t->endDate/10000;   e.month=(t->endDate/100)%100;   e.day=t->endDate%100;
        sprintf(ss,"%02d/%02d/%04d",s.day,s.month,s.year);
        sprintf(es,"%02d/%02d/%04d",e.day,e.month,e.year);
        printf("%-6d %-8d %-15s %-12s %-12s %-8d\n",
               t->waitId, t->customerId, t->model, ss, es, t->priority);
        t=t->next;
    }
}



void processWaitlistForCar(int carId) {
    int idx = findCarIdx(carId);
    if (idx==-1) return;
    WaitNode *cur=waitHead, *prev=NULL;
    while (cur) {
        if (strcmp(cur->model, cars[idx].model)==0 &&
            carAvailableForPeriod(carId, cur->startDate, cur->endDate)) {
            appendRental(cur->customerId, carId, cur->startDate, cur->endDate);
            recordModel(cars[idx].model);
            printf("Waitlist matched: Customer %d assigned Car %d (%s %s).\n",
                   cur->customerId, carId, cars[idx].brand, cars[idx].model);
            if (!prev) waitHead=cur->next;
            else prev->next=cur->next;
            free(cur);
            return;
        }
        prev=cur;
        cur=cur->next;
    }
}