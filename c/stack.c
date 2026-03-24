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