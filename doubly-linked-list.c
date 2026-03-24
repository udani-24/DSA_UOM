
typedef struct RecentCarNode {
    int   carId;
    char  brand[50];
    char  model[50];
    float pricePerDay;
    struct RecentCarNode *next;
} RecentCarNode;


Rental   *rentHead   = NULL;
Rental   *rentTail   = NULL;
int       nextRentId = 1;

CarCLL   *cllTail    = NULL;
FreqNode *freqHead   = NULL;

void appendRental(int custId, int carId, int start, int end) {
    Rental *n = malloc(sizeof(Rental));
    if (!n) { printf("Memory error.\n"); exit(1); }
    n->rentalId=nextRentId++;
    n->customerId=custId;
    n->carId=carId;
    n->startDate=start;
    n->endDate=end;
    n->returned=0;
    n->prev=NULL;
    n->next=NULL;
    if (!rentHead) { rentHead=rentTail=n; }
    else { rentTail->next=n; n->prev=rentTail; rentTail=n; }
}

Rental* findActiveRental(int carId) {
    Rental *t = rentHead;
    while (t) { if (t->carId==carId && t->returned==0) return t; t=t->next; }
    return NULL;
}

void displayRentalHistory() {
    Rental *t = rentHead;
    if (!t) { printf("No rental records.\n"); return; }
    printf("\n========== RENTAL HISTORY ==========\n");
    printf("%-6s %-8s %-8s %-12s %-12s %-10s\n",
           "RentID","CustID","CarID","Start","End","Status");
    printf("------------------------------------------------------------\n");
    while (t) {
        Date s, e;
        s.year=t->startDate/10000; s.month=(t->startDate/100)%100; s.day=t->startDate%100;
        e.year=t->endDate/10000;   e.month=(t->endDate/100)%100;   e.day=t->endDate%100;
        char ss[12], es[12];
        sprintf(ss,"%02d/%02d/%04d",s.day,s.month,s.year);
        sprintf(es,"%02d/%02d/%04d",e.day,e.month,e.year);
        const char *status = t->returned==0 ? "Active" :
                             t->returned==1 ? "Returned" : "Cancelled";
        printf("%-6d %-8d %-8d %-12s %-12s %-10s\n",
               t->rentalId, t->customerId, t->carId, ss, es, status);
        t = t->next;
    }
}
