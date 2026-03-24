#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

#define MAX_CARS   100
#define INPUT_BUF  100
#define MAX_RECENT 5



typedef struct { int day, month, year; } Date;

int dateToInt(Date d) { return d.year * 10000 + d.month * 100 + d.day; }

int parseDate(const char *s, Date *d) {
    int day, mon, yr;
    if (sscanf(s, "%d/%d/%d", &day, &mon, &yr) != 3) return 0;
    if (yr  < 2000 || yr  > 2100) return 0;
    if (mon < 1    || mon > 12  ) return 0;
    if (day < 1    || day > 31  ) return 0;
    d->day = day; d->month = mon; d->year = yr;
    return 1;
}

Date addDays(Date d, int n) {
    static int dim[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    d.day += n;
    while (1) {
        int leap = (d.year%4==0 && (d.year%100!=0 || d.year%400==0));
        int md   = (d.month==2 && leap) ? 29 : dim[d.month];
        if (d.day <= md) break;
        d.day -= md;
        d.month++;
        if (d.month > 12) { d.month = 1; d.year++; }
    }
    return d;
}

void printDate(Date d) { printf("%02d/%02d/%04d", d.day, d.month, d.year); }

int datesOverlap(int s1, int e1, int s2, int e2) {
    return s1 <= e2 && s2 <= e1;
}

typedef struct {
    int   carId;
    char  brand[50];
    char  model[50];
    float pricePerDay;
    int   active;
} Car;

typedef struct Customer {
    int             customerId;
    char            name[50];
    char            phone[20];
    char            licenseNo[30];
    struct Customer *next;
} Customer;

typedef struct Rental {
    int           rentalId;
    int           customerId;
    int           carId;
    int           startDate;
    int           endDate;
    int           returned;
    struct Rental *prev;
    struct Rental *next;
} Rental;

typedef struct CarCLL {
    int           carId;
    char          brand[50];
    char          model[50];
    float         pricePerDay;
    struct CarCLL *next;
} CarCLL;

typedef struct CarSLL {
    int           carId;
    char          brand[50];
    char          model[50];
    float         pricePerDay;
    int           active;
    struct CarSLL *next;
} CarSLL;

typedef struct FreqNode {
    char            model[50];
    int             count;
    struct FreqNode *next;
} FreqNode;

typedef struct WaitNode {
    int             waitId;
    int             customerId;
    char            model[50];
    int             startDate;
    int             endDate;
    int             priority;
    struct WaitNode *next;
} WaitNode;

typedef struct RecentCarNode {
    int   carId;
    char  brand[50];
    char  model[50];
    float pricePerDay;
    struct RecentCarNode *next;
} RecentCarNode;

typedef struct RecentCustNode {
    int  customerId;
    char name[50];
    char isVip;
    struct RecentCustNode *next;
} RecentCustNode;

Car       cars[MAX_CARS];
int       carCount   = 0;
int       nextCarId  = 1;

Customer *custHead   = NULL;
int       nextCustId = 1;

Rental   *rentHead   = NULL;
Rental   *rentTail   = NULL;
int       nextRentId = 1;

CarCLL   *cllTail    = NULL;
FreqNode *freqHead   = NULL;

WaitNode *waitHead   = NULL;
int       nextWaitId = 1;

RecentCarNode  *recentCarTop  = NULL;
RecentCustNode *recentCustTop = NULL;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

void waitEnter() {
    printf("\nPress Enter to return to menu...");
    getchar();
}

void readLine(char *buf, int size) {
    if (fgets(buf, size, stdin))
        buf[strcspn(buf, "\n")] = '\0';
}

int isExit(const char *s) {
    return strcmp(s,"E")==0 || strcmp(s,"e")==0;
}

void toLower(char *s) {
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

int rdInt(int *out) {
    char buf[INPUT_BUF];
    readLine(buf, INPUT_BUF);
    if (isExit(buf)) return 0;
    return sscanf(buf, "%d", out) == 1 ? 1 : -1;
}

int rdFloat(float *out) {
    char buf[INPUT_BUF];
    readLine(buf, INPUT_BUF);
    if (isExit(buf)) return 0;
    return sscanf(buf, "%f", out) == 1 ? 1 : -1;
}

int rdStr(char *out, int size) {
    readLine(out, size);
    if (isExit(out))      return 0;
    if (strlen(out) == 0) return -1;
    return 1;
}

int rdModel(char *out, int size) {
    int r = rdStr(out, size);
    if (r == 1) toLower(out);
    return r;
}

int rdBrand(char *out, int size) {
    readLine(out, size);
    if (isExit(out)) return 0;
    toLower(out);
    int i, len = (int)strlen(out);
    if (len < 2 || out[0]==' ' || out[len-1]==' ') return -1;
    for (i = 0; i < len; i++)
        if (!isalpha((unsigned char)out[i]) && out[i]!=' ') return -1;
    return 1;
}

int rdDate(Date *out) {
    char buf[INPUT_BUF];
    readLine(buf, INPUT_BUF);
    if (isExit(buf)) return 0;
    return parseDate(buf, out) ? 1 : -1;
}

int validPhone(const char *s) {
    int i, dc=0, len=(int)strlen(s);
    if (!len) return 0;
    for (i=0;i<len;i++) {
        if (isdigit((unsigned char)s[i])) dc++;
        else if (s[i]=='+'||s[i]=='-'||s[i]=='('||s[i]==')'||s[i]==' ') {}
        else return 0;
    }
    return dc>=7 && dc<=15;
}

int validLicense(const char *s) {
    int i, len=(int)strlen(s);
    if (len<4||len>20) return 0;
    for (i=0;i<len;i++)
        if (!isalnum((unsigned char)s[i]) && s[i]!='-') return 0;
    return 1;
}

int validName(const char *s) {
    int i, len=(int)strlen(s);
    if (len<2||s[0]==' '||s[len-1]==' ') return 0;
    for (i=0;i<len;i++)
        if (!isalpha((unsigned char)s[i]) && s[i]!=' ') return 0;
    return 1;
}

#define HANDLE(ret, msg) \
    if ((ret)==0) return; \
    if ((ret)==-1) { printf("  " msg "\n"); sleep_ms(700); clearScreen(); continue; }

void pushRecentCar(int carId, const char *brand, const char *model, float price) {
    RecentCarNode *cur = recentCarTop, *prev = NULL;
    while (cur) {
        if (cur->carId == carId) {
            if (prev) prev->next = cur->next;
            else      recentCarTop = cur->next;
            cur->next    = recentCarTop;
            recentCarTop = cur;
            strcpy(cur->brand, brand);
            strcpy(cur->model, model);
            cur->pricePerDay = price;
            return;
        }
        prev = cur; cur = cur->next;
    }
    RecentCarNode *n = malloc(sizeof(RecentCarNode));
    if (!n) { printf("Memory error.\n"); exit(1); }
    n->carId = carId; n->pricePerDay = price;
    strcpy(n->brand, brand); strcpy(n->model, model);
    n->next = recentCarTop; recentCarTop = n;

    int count = 1;
    RecentCarNode *t = recentCarTop;
    while (t->next) {
        count++;
        if (count == MAX_RECENT) {
            RecentCarNode *drop = t->next; t->next = NULL;
            while (drop) { RecentCarNode *d = drop; drop = drop->next; free(d); }
            break;
        }
        t = t->next;
    }
}

void pushRecentCust(int custId, const char *name, int isVip) {
    RecentCustNode *cur = recentCustTop, *prev = NULL;
    while (cur) {
        if (cur->customerId == custId) {
            if (prev) prev->next = cur->next;
            else      recentCustTop = cur->next;
            cur->next     = recentCustTop;
            recentCustTop = cur;
            strcpy(cur->name, name);
            cur->isVip = (char)isVip;
            return;
        }
        prev = cur; cur = cur->next;
    }
    RecentCustNode *n = malloc(sizeof(RecentCustNode));
    if (!n) { printf("Memory error.\n"); exit(1); }
    n->customerId = custId; n->isVip = (char)isVip;
    strcpy(n->name, name);
    n->next = recentCustTop; recentCustTop = n;

    int count = 1;
    RecentCustNode *t = recentCustTop;
    while (t->next) {
        count++;
        if (count == MAX_RECENT) {
            RecentCustNode *drop = t->next; t->next = NULL;
            while (drop) { RecentCustNode *d = drop; drop = drop->next; free(d); }
            break;
        }
        t = t->next;
    }
}
void displaySearchHistory(void) {
    int any = 0;

    printf("\n======= RECENT CAR SEARCHES (Most Recent First) =======\n");
    printf("%-6s %-8s %-15s %-15s %-10s\n", "No.", "Car ID", "Brand", "Model", "Price/Day");
    printf("------------------------------------------------------------\n");
    RecentCarNode *ct = recentCarTop;
    int ci = 1;
    while (ct && ci <= MAX_RECENT) {
        printf("[%-2d]  %-8d %-15s %-15s %.2f\n",
               ci, ct->carId, ct->brand, ct->model, ct->pricePerDay);
        ci++; ct = ct->next; any = 1;
    }
    if (!any) printf("  No car searches yet.\n");

    any = 0;
    printf("\n======= RECENT CUSTOMER SEARCHES (Most Recent First) =======\n");
    printf("%-6s %-10s %-22s\n", "No.", "Cust ID", "Name");
    printf("------------------------------------------\n");
    RecentCustNode *kt = recentCustTop;
    int ki = 1;
    while (kt && ki <= MAX_RECENT) {
        printf("[%-2d]  %-10d %-22s\n", ki, kt->customerId, kt->name);
        ki++; kt = kt->next; any = 1;
    }
    if (!any) printf("  No customer searches yet.\n");

    printf("\n------------------------------------------------------------\n");
    printf("Enter C<n> for car details or K<n> for customer details.\n");
    printf("Press Enter to go back. Examples: C1  K2\n");
    printf("Choice: ");

    char buf[INPUT_BUF];
    readLine(buf, INPUT_BUF);
    if (strlen(buf) == 0 || isExit(buf)) return;

    char type = (char)toupper((unsigned char)buf[0]);
    int  idx  = 0;
    if (strlen(buf) >= 2) sscanf(buf + 1, "%d", &idx);

    if (type == 'C' && idx >= 1 && idx < ci) {
        RecentCarNode *node = recentCarTop;
        int i = 1;
        while (node && i < idx) { node = node->next; i++; }
        if (node) {
            printf("\n--- Car Details ---\n");
            printf("  Car ID    : %d\n", node->carId);
            printf("  Brand     : %s\n", node->brand);
            printf("  Model     : %s\n", node->model);
            printf("  Price/Day : %.2f\n", node->pricePerDay);
            int k;
            for (k = 0; k < carCount; k++) {
                if (cars[k].carId == node->carId) {
                    printf("  Status    : %s\n", cars[k].active ? "In Fleet" : "Removed");
                    break;
                }
            }
        }
    } else if (type == 'K' && idx >= 1 && idx < ki) {
        RecentCustNode *node = recentCustTop;
        int i = 1;
        while (node && i < idx) { node = node->next; i++; }
        if (node) {
            Customer *c = custHead;
            while (c && c->customerId != node->customerId) c = c->next;
            printf("\n--- Customer Details ---\n");
            if (c) {
                printf("  Cust ID  : %d\n", c->customerId);
                printf("  Name     : %s\n", c->name);
                printf("  Phone    : %s\n", c->phone);
                printf("  License  : %s\n", c->licenseNo);
            } else {
                printf("  Cust ID  : %d\n", node->customerId);
                printf("  Name     : %s\n", node->name);
                printf("  (Customer record no longer exists)\n");
            }
        }
    } else {
        printf("  Invalid selection.\n");
    }
}


void cllInsert(Car *c) {
    CarCLL *n = malloc(sizeof(CarCLL));
    if (!n) { printf("Memory error.\n"); exit(1); }
    n->carId = c->carId;
    n->pricePerDay = c->pricePerDay;
    strcpy(n->brand, c->brand);
    strcpy(n->model, c->model);
    if (!cllTail) { n->next = n; cllTail = n; }
    else { n->next = cllTail->next; cllTail->next = n; cllTail = n; }
}

int findCarIdx(int carId) {
    int i;
    for (i=0;i<carCount;i++)
        if (cars[i].carId==carId && cars[i].active) return i;
    return -1;
}

int findAnyCarIdx(int carId) {
    int i;
    for (i=0;i<carCount;i++)
        if (cars[i].carId==carId) return i;
    return -1;
}

int carAvailableForPeriod(int carId, int start, int end) {
    Rental *r = rentHead;
    while (r) {
        if (r->carId==carId && r->returned==0)
            if (datesOverlap(r->startDate, r->endDate, start, end))
                return 0;
        r = r->next;
    }
    return 1;
}

void addCar() {
    while (1) {
        Car tmp; int ret;
        if (carCount >= MAX_CARS) { printf("Fleet is full.\n"); return; }
        printf("\n--- Add Car (E to go back) ---\n");
        tmp.carId = nextCarId;
        printf("Auto Car ID: %d\n", tmp.carId);

        printf("Brand: ");
        ret = rdBrand(tmp.brand, sizeof(tmp.brand));
        HANDLE(ret, "Invalid brand. Letters and spaces only, min 2 chars.");

        printf("Model: ");
        ret = rdModel(tmp.model, sizeof(tmp.model));
        HANDLE(ret, "Model cannot be empty.");

        printf("Price Per Day: ");
        ret = rdFloat(&tmp.pricePerDay);
        HANDLE(ret, "Invalid price.");
        if (tmp.pricePerDay <= 0) {
            printf("  Price must be positive.\n");
            sleep_ms(700); clearScreen(); continue;
        }

        tmp.active = 1;
        cars[carCount++] = tmp;
        nextCarId++;
        cllInsert(&tmp);
        printf("Car added. ID=%d | %s %s | %.2f/day\n",
               tmp.carId, tmp.brand, tmp.model, tmp.pricePerDay);
        return;
    }
}

void removeCar() {
    while (1) {
        int id, idx, ret;
        printf("\n--- Remove Car from Fleet (E to go back) ---\n");
        printf("Car ID: ");
        ret = rdInt(&id);
        HANDLE(ret, "Invalid input.");

        idx = findCarIdx(id);
        if (idx==-1) { printf("  Car not found.\n"); sleep_ms(700); clearScreen(); continue; }

        Rental *r = rentHead; int busy = 0;
        while (r) { if (r->carId==id && r->returned==0) { busy=1; break; } r=r->next; }
        if (busy) {
            printf("  Cannot remove -- car has an active rental.\n");
            sleep_ms(700); clearScreen(); continue;
        }
        cars[idx].active = 0;
        printf("Car %d removed from fleet.\n", id);
        return;
    }
}

void displayFleet() {
    int i, any=0;
    printf("\n========== FLEET ==========\n");
    printf("%-6s %-12s %-15s %-10s\n", "ID", "Brand", "Model", "Price/Day");
    printf("--------------------------------------------------\n");
    for (i=0;i<carCount;i++) {
        if (!cars[i].active) continue;
        printf("%-6d %-12s %-15s %.2f\n",
               cars[i].carId, cars[i].brand, cars[i].model, cars[i].pricePerDay);
        any=1;
    }
    if (!any) printf("Fleet is empty.\n");
}

void displayAvailableCars() {
    int i, found=0;
    printf("\n========== AVAILABLE CARS (No Active Rental) ==========\n");
    printf("%-6s %-12s %-15s %-10s\n", "ID", "Brand", "Model", "Price/Day");
    printf("--------------------------------------------------\n");
    for (i=0;i<carCount;i++) {
        if (!cars[i].active) continue;
        Rental *r = rentHead; int busy=0;
        while (r) { if (r->carId==cars[i].carId && r->returned==0){busy=1;break;} r=r->next; }
        if (!busy) {
            printf("%-6d %-12s %-15s %.2f\n",
                   cars[i].carId, cars[i].brand, cars[i].model, cars[i].pricePerDay);
            found=1;
        }
    }
    if (!found) printf("No cars currently available.\n");
}

CarSLL* buildCarSLL() {
    int i;
    CarSLL *head = NULL, *tail = NULL;
    for (i = 0; i < carCount; i++) {
        if (!cars[i].active) continue;
        CarSLL *n = malloc(sizeof(CarSLL));
        if (!n) { printf("Memory error.\n"); exit(1); }
        n->carId       = cars[i].carId;
        n->pricePerDay = cars[i].pricePerDay;
        n->active      = cars[i].active;
        strcpy(n->brand, cars[i].brand);
        strcpy(n->model, cars[i].model);
        n->next = NULL;
        if (!head) { head = tail = n; }
        else       { tail->next = n; tail = n; }
    }
    return head;
}

void freeCarSLL(CarSLL *head) {
    while (head) {
        CarSLL *tmp = head;
        head = head->next;
        free(tmp);
    }
}

void bubbleSortSLLByPrice(CarSLL *head) {
    int swapped;
    CarSLL *cur;
    if (!head) return;
    do {
        swapped = 0; cur = head;
        while (cur->next) {
            if (cur->pricePerDay > cur->next->pricePerDay) {
                int   ti = cur->carId;        cur->carId        = cur->next->carId;        cur->next->carId        = ti;
                float tp = cur->pricePerDay;  cur->pricePerDay  = cur->next->pricePerDay;  cur->next->pricePerDay  = tp;
                int   ta = cur->active;       cur->active       = cur->next->active;       cur->next->active       = ta;
                char  tb[50], tm[50];
                strcpy(tb, cur->brand); strcpy(cur->brand, cur->next->brand); strcpy(cur->next->brand, tb);
                strcpy(tm, cur->model); strcpy(cur->model, cur->next->model); strcpy(cur->next->model, tm);
                swapped = 1;
            }
            cur = cur->next;
        }
    } while (swapped);
}

void bubbleSortSLLByBrand(CarSLL *head) {
    int swapped;
    CarSLL *cur;
    if (!head) return;
    do {
        swapped = 0; cur = head;
        while (cur->next) {
            if (strcmp(cur->brand, cur->next->brand) > 0) {
                int   ti = cur->carId;        cur->carId        = cur->next->carId;        cur->next->carId        = ti;
                float tp = cur->pricePerDay;  cur->pricePerDay  = cur->next->pricePerDay;  cur->next->pricePerDay  = tp;
                int   ta = cur->active;       cur->active       = cur->next->active;       cur->next->active       = ta;
                char  tb[50], tm[50];
                strcpy(tb, cur->brand); strcpy(cur->brand, cur->next->brand); strcpy(cur->next->brand, tb);
                strcpy(tm, cur->model); strcpy(cur->model, cur->next->model); strcpy(cur->next->model, tm);
                swapped = 1;
            }
            cur = cur->next;
        }
    } while (swapped);
}

void displayCarSLL(CarSLL *head) {
    int rank = 1;
    if (!head) { printf("No cars to display.\n"); return; }
    printf("%-5s %-6s %-12s %-15s %-10s\n", "Rank","ID","Brand","Model","Price/Day");
    printf("--------------------------------------------------\n");
    while (head) {
        printf("%-5d %-6d %-12s %-15s %.2f\n",
               rank++, head->carId, head->brand, head->model, head->pricePerDay);
        head = head->next;
    }
}

void sortCarsByPrice() {
    if (!carCount) { printf("No cars in fleet.\n"); return; }
    printf("\n====== CARS SORTED BY PRICE (Low to High) ======\n");
    CarSLL *sll = buildCarSLL();
    bubbleSortSLLByPrice(sll);
    displayCarSLL(sll);
    freeCarSLL(sll);
}

void sortCarsByBrand() {
    if (!carCount) { printf("No cars in fleet.\n"); return; }
    printf("\n====== CARS SORTED BY BRAND (A to Z) ======\n");
    CarSLL *sll = buildCarSLL();
    bubbleSortSLLByBrand(sll);
    displayCarSLL(sll);
    freeCarSLL(sll);
}

void searchCarById() {
    while (1) {
        int id, ret;
        printf("\n--- Search Car by ID (E to go back) ---\n");
        printf("Car ID: ");
        ret = rdInt(&id);
        HANDLE(ret, "Invalid input.");

        if (!cllTail) { printf("  Fleet is empty.\n"); sleep_ms(700); clearScreen(); continue; }

        CarCLL *head=cllTail->next, *cur=head, *found=NULL;
        do {
            if (cur->carId==id) { found=cur; break; }
            cur=cur->next;
        } while (cur!=head);

        if (!found) { printf("  Car not found.\n"); sleep_ms(700); clearScreen(); continue; }

        int idx=findCarIdx(id);
        printf("Found: ID=%d | %s | %s | %.2f/day | %s\n",
               found->carId, found->brand, found->model, found->pricePerDay,
               (idx!=-1) ? "In Fleet" : "Removed");
        pushRecentCar(found->carId, found->brand, found->model, found->pricePerDay);
        return;
    }
}


Customer* findCustById(int id) {
    Customer *t = custHead;
    while (t) { if (t->customerId==id) return t; t=t->next; }
    return NULL;
}

Customer* createOrFindCustomer() {
    while (1) {
        int id, ret;
        printf("Customer ID (0 to create new): ");
        ret = rdInt(&id);
        if (ret==0) return NULL;
        if (ret==-1) { printf("  Invalid input.\n"); sleep_ms(700); continue; }

        if (id != 0) {
            Customer *c = findCustById(id);
            if (!c) {
                printf("  Customer not found. Try again or enter 0 to create new.\n");
                sleep_ms(700); continue;
            }
            pushRecentCust(c->customerId, c->name, 0);
            return c;
        }

        char name[50], phone[20], license[30];
        printf("Name: "); readLine(name, sizeof(name));
        if (!validName(name)) { printf("  Invalid name.\n"); sleep_ms(700); continue; }

        printf("Phone: "); readLine(phone, sizeof(phone));
        if (!validPhone(phone)) { printf("  Invalid phone.\n"); sleep_ms(700); continue; }

        printf("License No: "); readLine(license, sizeof(license));
        if (!validLicense(license)) { printf("  Invalid license.\n"); sleep_ms(700); continue; }

        Customer *n = malloc(sizeof(Customer));
        if (!n) { printf("Memory error.\n"); exit(1); }
        n->customerId = nextCustId++;
        strcpy(n->name, name);
        strcpy(n->phone, phone);
        strcpy(n->licenseNo, license);
        n->next = NULL;

        if (!custHead) custHead = n;
        else { Customer *t2 = custHead; while (t2->next) t2=t2->next; t2->next = n; }

        printf("Customer created. ID=%d | %s\n", n->customerId, n->name);
        pushRecentCust(n->customerId, n->name, 0);
        return n;
    }
}

void addCustomer() {
    while (1) {
        char name[50], phone[20], license[30];
        printf("\n--- Add Customer (E to go back) ---\n");

        printf("Name: "); readLine(name, sizeof(name));
        if (isExit(name)) return;
        if (!validName(name)) { printf("  Invalid name.\n"); sleep_ms(700); clearScreen(); continue; }

        printf("Phone: "); readLine(phone, sizeof(phone));
        if (isExit(phone)) return;
        if (!validPhone(phone)) { printf("  Invalid phone.\n"); sleep_ms(700); clearScreen(); continue; }

        printf("License No: "); readLine(license, sizeof(license));
        if (isExit(license)) return;
        if (!validLicense(license)) { printf("  Invalid license.\n"); sleep_ms(700); clearScreen(); continue; }

        Customer *n = malloc(sizeof(Customer));
        if (!n) { printf("Memory error.\n"); exit(1); }
        n->customerId = nextCustId++;
        strcpy(n->name, name);
        strcpy(n->phone, phone);
        strcpy(n->licenseNo, license);
        n->next = NULL;

        if (!custHead) custHead = n;
        else { Customer *t = custHead; while (t->next) t=t->next; t->next = n; }

        printf("Customer added. ID=%d | %s\n", n->customerId, n->name);
        pushRecentCust(n->customerId, n->name, 0);
        return;
    }
}

void displayCustomers() {
    Customer *t = custHead;
    if (!t) { printf("No customers on record.\n"); return; }
    printf("\n========== CUSTOMERS ==========\n");
    printf("%-6s %-20s %-18s %-20s\n", "ID", "Name", "Phone", "License");
    printf("------------------------------------------------------------\n");
    while (t) {
        printf("%-6d %-20s %-18s %-20s\n",
               t->customerId, t->name, t->phone, t->licenseNo);
        t = t->next;
    }
}

void removeCustomer() {
    while (1) {
        int id, ret;
        printf("\n--- Remove Customer (E to go back) ---\n");
        printf("Customer ID: ");
        ret = rdInt(&id);
        HANDLE(ret, "Invalid input.");

        Customer *c = findCustById(id);
        if (!c) { printf("  Customer not found.\n"); sleep_ms(700); clearScreen(); continue; }

        int busy = 0;
        Rental *r = rentHead;
        while (r) { if (r->customerId==id && r->returned==0){busy=1;break;} r=r->next; }
        if (busy) {
            printf("  Cannot remove -- customer has an active rental.\n");
            sleep_ms(700); clearScreen(); continue;
        }

        Customer *prev = NULL, *t = custHead;
        while (t && t->customerId != id) { prev = t; t = t->next; }
        if (prev) prev->next = t->next;
        else      custHead   = t->next;
        free(t);
        printf("Customer %d removed.\n", id);
        return;
    }
}

void searchCustomerById() {
    while (1) {
        int id, ret;
        printf("\n--- Search Customer by ID (E to go back) ---\n");
        printf("Customer ID: ");
        ret = rdInt(&id);
        HANDLE(ret, "Invalid input.");
        Customer *c = findCustById(id);
        if (!c) { printf("  Customer not found.\n"); sleep_ms(700); clearScreen(); continue; }
        printf("Found: ID=%d | %s | %s | %s\n",
               c->customerId, c->name, c->phone, c->licenseNo);
        pushRecentCust(c->customerId, c->name, 0);
        return;
    }
}

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

void recordModel(const char *model) {
    FreqNode *t = freqHead;
    while (t) {
        if (strcmp(t->model,model)==0) { t->count++; return; }
        t=t->next;
    }
    FreqNode *n = malloc(sizeof(FreqNode));
    if (!n) { printf("Memory error.\n"); exit(1); }
    strcpy(n->model, model);
    n->count=1;
    n->next=freqHead;
    freqHead=n;
}

void bubbleSortFreq() {
    if (!freqHead) return;
    int swapped;
    FreqNode *cur;
    do {
        swapped=0;
        cur=freqHead;
        while (cur->next) {
            if (cur->count < cur->next->count) {
                char tmp[50];
                int tc;
                tc=cur->count;
                cur->count=cur->next->count;
                cur->next->count=tc;
                strcpy(tmp,cur->model);
                strcpy(cur->model,cur->next->model);
                strcpy(cur->next->model,tmp);
                swapped=1;
            }
            cur=cur->next;
        }
    } while (swapped);
}

void displayModelReport() {
    if (!freqHead) { printf("No rental data yet.\n"); return; }
    bubbleSortFreq();
    printf("\n====== MOST RENTED MODELS ======\n");
    printf("%-6s %-20s %s\n","Rank","Model","Rentals");
    printf("--------------------------------\n");
    FreqNode *t=freqHead; int rank=1;
    while (t) {
        printf("%-6d %-20s %d\n", rank++, t->model, t->count);
        t=t->next;
    }
    printf("--------------------------------\n");
    printf("Top model: %s -- consider expanding this in your fleet.\n", freqHead->model);
}

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

void rentCar() {
    while (1) {
        char model[50];
        Date startD;
        int days, ret, i;

        printf("\n--- Rent Car (E to go back) ---\n");

        printf("Car model requested: ");
        ret = rdModel(model, sizeof(model));
        HANDLE(ret, "Model cannot be empty.");

        printf("Start date (DD/MM/YYYY): ");
        ret = rdDate(&startD);
        HANDLE(ret, "Invalid date. Use DD/MM/YYYY.");

        printf("Number of days: ");
        ret = rdInt(&days);
        HANDLE(ret, "Invalid number.");
        if (days<=0) {
            printf("  Days must be positive.\n");
            sleep_ms(700); clearScreen(); continue;
        }

        Date endD   = addDays(startD, days-1);
        int  startI = dateToInt(startD);
        int  endI   = dateToInt(endD);

        printf("\n  Available '%s' cars from ", model);
        printDate(startD); printf(" to "); printDate(endD); printf(":\n\n");

        int found=0;
        for (i=0;i<carCount;i++) {
            if (!cars[i].active) continue;
            if (strcmp(cars[i].model,model)!=0) continue;
            if (!carAvailableForPeriod(cars[i].carId,startI,endI)) continue;
            printf("  Car ID:%-4d | %s %s | %.2f/day\n",
                   cars[i].carId, cars[i].brand, cars[i].model, cars[i].pricePerDay);
            found=1;
        }

        if (!found) {
            char ans[10];
            printf("  No '%s' cars available for that period.\n\n", model);
            printf("Would you like to join the priority waitlist? (y/n): ");
            readLine(ans, sizeof(ans));

            if (ans[0]=='y'||ans[0]=='Y') {
                Customer *c = createOrFindCustomer();
                if (!c) return;
                int priority;
                printf("Enter priority (1=high, 2=medium, 3=low): ");
                ret = rdInt(&priority);
                if (ret!=1||priority<1||priority>3) {
                    printf("  Invalid priority.\n");
                    sleep_ms(700); clearScreen(); continue;
                }
                enqueueWaitlist(c->customerId, model, startI, endI, priority);
                printf("Customer added to waitlist successfully.\n");
                return;
            }

            printf("  All cars available for that period:\n\n");
            int anyOther=0;
            for (i=0;i<carCount;i++) {
                if (!cars[i].active) continue;
                if (!carAvailableForPeriod(cars[i].carId,startI,endI)) continue;
                printf("  Car ID:%-4d | %-12s | %-15s | %.2f/day\n",
                       cars[i].carId, cars[i].brand, cars[i].model, cars[i].pricePerDay);
                anyOther=1;
            }
            if (!anyOther) printf("  No cars available at all for that period.\n");

            printf("\nEnter a Car ID from the list above to rent instead, or press E to go back: ");
            int carId=-1; ret=rdInt(&carId);
            if (ret!=1) return;

            int idx=findCarIdx(carId);
            if (idx==-1) {
                printf("  Car not found.\n");
                sleep_ms(700); clearScreen(); continue;
            }
            if (!carAvailableForPeriod(carId,startI,endI)) {
                printf("  That car is not available.\n");
                sleep_ms(700); clearScreen(); continue;
            }
            Customer *c = createOrFindCustomer();
            if (!c) return;
            pushRecentCar(cars[idx].carId,cars[idx].brand,cars[idx].model,cars[idx].pricePerDay);
            float total=cars[idx].pricePerDay*days;
            appendRental(c->customerId,carId,startI,endI);
            recordModel(cars[idx].model);
            printf("\n  Rental confirmed.\n");
            printf("  Customer : %s (ID %d)\n", c->name, c->customerId);
            printf("  Car      : %s %s (ID %d)\n", cars[idx].brand, cars[idx].model, carId);
            printf("  Period   : "); printDate(startD); printf(" to "); printDate(endD);
            printf(" (%d days)\n", days);
            printf("  Total    : %.2f\n", total);
            return;
        }

        int carId=-1;
        printf("\nEnter Car ID to rent: ");
        ret=rdInt(&carId);
        if (ret!=1) return;

        int idx=findCarIdx(carId);
        if (idx==-1||strcmp(cars[idx].model,model)!=0) {
            printf("  Invalid car ID for that model.\n");
            sleep_ms(700); clearScreen(); continue;
        }
        if (!carAvailableForPeriod(carId,startI,endI)) {
            printf("  That car is not available.\n");
            sleep_ms(700); clearScreen(); continue;
        }

        Customer *c = createOrFindCustomer();
        if (!c) return;
        pushRecentCar(cars[idx].carId,cars[idx].brand,cars[idx].model,cars[idx].pricePerDay);
        {
            float total=cars[idx].pricePerDay*days;
            appendRental(c->customerId,carId,startI,endI);
            recordModel(model);
            printf("\n  Rental confirmed.\n");
            printf("  Customer : %s (ID %d)\n", c->name, c->customerId);
            printf("  Car      : %s %s (ID %d)\n", cars[idx].brand, cars[idx].model, carId);
            printf("  Period   : "); printDate(startD); printf(" to "); printDate(endD);
            printf(" (%d days)\n", days);
            printf("  Total    : %.2f\n", total);
        }
        return;
    }
}

void returnCar() {
    while (1) {
        int carId, ret;
        printf("\n--- Return Car (E to go back) ---\n");
        printf("Car ID: ");
        ret = rdInt(&carId);
        HANDLE(ret, "Invalid input.");

        int idx=findAnyCarIdx(carId);
        if (idx==-1) {
            printf("  Car not found.\n");
            sleep_ms(700); clearScreen(); continue;
        }

        Rental *r=findActiveRental(carId);
        if (!r) {
            printf("  No active rental for that car.\n");
            sleep_ms(700); clearScreen(); continue;
        }

        r->returned=1;
        pushRecentCar(cars[idx].carId,cars[idx].brand,cars[idx].model,cars[idx].pricePerDay);
        printf("Car %d returned. Rental #%d closed.\n", carId, r->rentalId);

        if (cars[idx].active) processWaitlistForCar(carId);
        return;
    }
}

void cancelRental() {
    while (1) {
        int rentalId, ret;
        printf("\n--- Cancel Active Rental (E to go back) ---\n");
        printf("Rental ID to cancel: ");
        ret = rdInt(&rentalId);
        HANDLE(ret, "Invalid input.");

        Rental *r=rentHead;
        while (r) { if (r->rentalId==rentalId) break; r=r->next; }

        if (!r) {
            printf("  Rental not found.\n");
            sleep_ms(700); clearScreen(); continue;
        }
        if (r->returned!=0) {
            printf("  Rental is already closed.\n");
            sleep_ms(700); clearScreen(); continue;
        }

        r->returned=2;
        printf("Rental #%d cancelled.\n", rentalId);

        {
            int idx=findAnyCarIdx(r->carId);
            if (idx!=-1 && cars[idx].active) processWaitlistForCar(r->carId);
        }
        return;
    }
}

void mainMenu() {
    int choice, ret;
    do {
        clearScreen();
        printf("\n============================================\n");
        printf("           CAR RENTAL SYSTEM\n");
        printf("============================================\n");
        printf("  FLEET\n");
        printf("   1. Add Car\n");
        printf("   2. Remove Car\n");
        printf("   3. Display Fleet\n");
        printf("   4. Display Available Cars\n");
        printf("   5. Search Car by ID\n");
        printf("   6. Sort Cars by Price\n");
        printf("   7. Sort Cars by Brand\n");
        printf("--------------------------------------------\n");
        printf("  CUSTOMERS\n");
        printf("   8. Add Customer\n");
        printf("   9. Display Customers\n");
        printf("  10. Remove Customer\n");
        printf("  11. Search Customer by ID\n");
        printf("--------------------------------------------\n");
        printf("  RENTALS\n");
        printf("  12. Rent Car\n");
        printf("  13. Return Car\n");
        printf("  14. Cancel Rental\n");
        printf("  15. Rental History\n");
        printf("--------------------------------------------\n");
        printf("  WAITLIST / REPORTS\n");
        printf("  16. Display Priority Waitlist\n");
        printf("  17. Most Rented Models\n");
        printf("  18. Recent Search History\n");
        printf("--------------------------------------------\n");
        printf("   0. Exit\n");
        printf("============================================\n");
        printf("Choice: ");

        ret = rdInt(&choice);
        if (ret != 1) { printf("Invalid.\n"); sleep_ms(500); continue; }
        clearScreen();

        switch (choice) {
            case  1: addCar();               break;
            case  2: removeCar();            break;
            case  3: displayFleet();         break;
            case  4: displayAvailableCars(); break;
            case  5: searchCarById();        break;
            case  6: sortCarsByPrice();      break;
            case  7: sortCarsByBrand();      break;
            case  8: addCustomer();          break;
            case  9: displayCustomers();     break;
            case 10: removeCustomer();       break;
            case 11: searchCustomerById();   break;
            case 12: rentCar();              break;
            case 13: returnCar();            break;
            case 14: cancelRental();         break;
            case 15: displayRentalHistory(); break;
            case 16: displayWaitlist();      break;
            case 17: displayModelReport();   break;
            case 18: displaySearchHistory(); break;
            case  0: printf("Goodbye.\n"); return;
            default: printf("Invalid choice.\n"); sleep_ms(500);
        }
        if (choice != 0) waitEnter();
    } while (1);
}

int main() {
    mainMenu();
    return 0;
}
