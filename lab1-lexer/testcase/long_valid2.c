extern int debugLevel;

struct CacheNode {
    char key[50];
    //char value[16];
    struct ipValue* value;
    struct CacheNode* next;
    struct CacheNode* prev;
};

struct Hash {
    struct CacheNode* node;
    struct Hash* next;
};

void freeValue(struct ipValue* value) {
    struct ipValue* curPtr = value;
    while (curPtr) {
        struct ipValue* temp = curPtr->nextval;
        free(curPtr);
        curPtr = temp;
    }
}

void copyValue(struct ipValue* des, struct ipValue* src) {
    if (des == src) 
        return;
    else {
        if(des->nextval)
            free(des->nextval);
        struct ipValue* lhs = des, *rhs = src;
        while (rhs->nextval) {
            rhs = rhs->nextval;
            lhs->nextval = (struct ipValue*)malloc(sizeof(struct ipValue));
            if (lhs) {
                lhs = lhs->nextval;
                strcpy(lhs->value, rhs->value);
                lhs->nextval = NULL;
            }
            else 
                printf("Fail to allocate memory!\n");
        }
    }
}

void printValue(struct ipValue* ipval) {
    while (ipval->nextval) {
        ipval = ipval->nextval;
        printf("\t%s\n", ipval->value);
    }
}

unsigned hash(Cache* cache, const char* str) {
    unsigned hash_value = 0, len = strlen(str);
    for (int i = 0; i < len; ++i) {
        hash_value += str[i];
        hash_value += (hash_value << 10);
        hash_value ^= (hash_value >> 5);
    }
    hash_value += (hash_value << 3);
    hash_value ^= (hash_value >> 11);
    hash_value += (hash_value << 15);
    return hash_value % cache->capacity;
}

Cache* CacheInit(int capacity) {
    Cache* cache = (Cache*)malloc(sizeof(Cache));
    if (cache == NULL) {
        printf("Cannot allocate memory!\n");
        return NULL;
    }
    else {
        cache->cached = 0;
        cache->capacity = capacity;
        cache->table = (struct Hash*)malloc(sizeof(struct Hash) * capacity);
        if (cache->table == NULL) {
            printf("Cannot allocate memory!\n");
            return NULL;
        }
        for (int i = 0; i < capacity; ++i) {
          (cache->table)[i].node = NULL;
          (cache->table)[i].next = NULL;
        }

        cache->head = (struct CacheNode*)malloc(sizeof(struct CacheNode));
        if (cache->head != NULL) {
            cache->head->next = NULL;
            cache->tail = cache->head;
        }
        else {
            printf("Cannot allocate memory!\n");
        }
    }
    return cache;
}

void DeleteNode(Cache* cache, struct CacheNode* node) {
    if (node) {
        if(debugLevel == 2)
            printf("node to delete is: %s\n", node->key);
    }
    else return;
    if (node == cache->tail) {
        cache->tail = node->prev;
        node->prev->next = NULL;
        //DeleteHashNode(cache, node);
        cache->cached--;
        freeValue(node->value);
        free(node);
    }
    else {
        if (node->next == NULL) {
            printf("invalid! next node is null!\n");
            return;
        }
        struct CacheNode* prePtr = node->prev;
        struct CacheNode* nxtPtr = node->next;
        prePtr->next = nxtPtr;
        nxtPtr->prev = prePtr;
        cache->cached--;
        freeValue(node->value);
        free(node);
    }
}

struct CacheNode* CreateNode(Cache* cache, char* domain, struct ipValue* ipval) {
    //printf("WATCHING DOMAIN: %s\n", domain);
    if (cache->cached == cache->capacity) {
        //printf("CACHE IS FULL, free a member from cache!\n");
        // remember to delete from hash!!!!!
        unsigned index = hash(cache, cache->tail->key);
        struct Hash* pre = &(cache->table)[index];
        //struct Hash* head = pre;
        struct Hash* iter = pre->next;
        for (; iter; iter = iter->next, pre = pre->next)
            if (iter->node == cache->tail)
                break;
        pre->next = iter->next;
        free(iter);

        DeleteNode(cache, cache->tail);
    }
    struct CacheNode* node = (struct CacheNode*)malloc(sizeof(struct CacheNode));
    if (node) {
        if (cache->cached == 0) cache->tail = node;
        strcpy(node->key, domain);
        node->value = (struct ipValue*)malloc(sizeof(struct ipValue));
        node->value->nextval = NULL;
        copyValue(node->value, ipval);
        node->prev = cache->head;
        node->next = cache->head->next;
        if (node->next)
            node->next->prev = node;
        cache->head->next = node;
        cache->cached++;
    }
    return node;
}

struct Hash* CheckDomainExist(Cache* cache, char* domain) {
    unsigned index = hash(cache, domain);
    //printf("handling index: %d\n", index);
    struct Hash* iter = (cache->table)[index].next;
    for (; iter; iter = iter->next) {
        if (strcmp(domain, iter->node->key) == 0) {
            //printf("target found: PTR=%p\n", iter);
            return iter;
        }
    }
    return NULL;
}

void CacheSet(Cache* cache, char* domain, struct ipValue* ipval) {
    if (CheckDomainExist(cache, domain))
        return;
    unsigned index = hash(cache, domain);
    if(debugLevel == 2)
        printf("%s hash successful, value=%d\n", domain, index);
    struct Hash *HashPtr = &(cache->table)[index], *curPtr;
    curPtr = (struct Hash*)malloc(sizeof(struct Hash));
    if (!curPtr) {
        printf("Cannot allocate memory!\n");
        return;
    }
    curPtr->node = CreateNode(cache, domain, ipval);
    curPtr->next = NULL;
    while(HashPtr->next) HashPtr = HashPtr->next;
    HashPtr->next = curPtr;
}

struct CacheNode* PushFront(Cache* cache, struct CacheNode* node) {
    char dom[50];
    strcpy(dom, node->key);
    struct ipValue* value = (struct ipValue*)malloc(sizeof(struct ipValue));
    value->nextval = NULL;
    copyValue(value, node->value);
    DeleteNode(cache, node);
    struct CacheNode* res = CreateNode(cache, dom, value);
    freeValue(value);
    return res;
}

int CacheGet(Cache* cache, char* domain, struct ipValue* ipval) {
    int found = 0;
    struct Hash* iter = CheckDomainExist(cache, domain);
    if (iter) {
        iter->node = PushFront(cache, iter->node);
        copyValue(ipval, iter->node->value);
        found = 1;
    }
    return found;
}

void print(Cache* cache) {
    printf("\n****************\n");
    printf("CACHED NUMBER: %d\n", cache->cached);
    for (struct CacheNode* cur = cache->head->next; cur; cur = cur->next) {
        printf("PTR = %p\n", cur);
        printf("%s:\n", cur->key);
        printValue(cur->value);
        if(cur->prev && cur->prev != cache->head)
            printf(", prev is: %s\n", cur->prev->key);
        else if(cur->prev && cur->prev == cache->head)
            printf(", prev is head\n");
        else 
            printf(", prev is NULL!\n");
    }
    printf("-------now checking hash table------\n");
    for(int i = 0; i < cache->capacity; ++i) {
        if((cache->table)[i].next) {
            for(struct Hash* cur = (cache->table)[i].next; cur; cur = cur->next) {
                printf("i = %d, content is %s, %p\n", i, cur->node->key, cur);
            }
        }
    }
    printf("*****************\n\n");
}