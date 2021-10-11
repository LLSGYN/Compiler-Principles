//引用main.cpp的全局变量
extern char filepath[50];//用户自己定义的配置文件地址
extern char dnsServerIP[16];//用户自己定义的dns服务器IP地址
extern int debugLevel;//调试等级，默认为0，-d为1，-dd为2

int judgeIPorPath(const char* temp)//判断字符串是dns服务器IP地址还是配置文件路径
{
	int len = strlen(temp);
	for (int i = 0; i < len;)
	{
		if (temp[i] == '.' || (temp[i] >= '0'&&temp[i] <= '9'))//IP地址由数字和.组成
			i++;
		else
			return 1;//表示文件路径
	}
	return 0;//表示IP地址
}

void paraIns(int argc, char** argv)//分析指令，更改参数
{
	switch (argc) {
	case 1:
		break;//dnsrelay
	case 2:             //dnsrelay [-d| -dd]/[dns-server-ipaddr]/[filename] //3选1
		if (strcmp(argv[1], "-d") == 0)//更改调试等级
			debugLevel = 1;
		else if (strcmp(argv[1], "-dd") == 0)
			debugLevel = 2;
		else if (judgeIPorPath(argv[1]) == 0)//传入指定IP地址
			strcpy(dnsServerIP, argv[1]);
		else
			strcpy(filepath, argv[1]);//传入指定的文件路径
		break;
	case 3:           //dnsrelay [-d| -dd]/[dns-server-ipaddr]/[filename] //3选2
		if (strcmp(argv[1], "-d") == 0)//更改调试等级
			debugLevel = 1;
		else if (strcmp(argv[1], "-dd") == 0)
			debugLevel = 2;
		else
			strcpy(dnsServerIP, argv[1]);//第一个参数为IP地址
		if (judgeIPorPath(argv[2]) == 1)//第二个参数为文件路径
			strcpy(filepath, argv[2]);
		else//第二个参数为IP地址
			strcpy(dnsServerIP, argv[2]);
		break;
	case 4:     //全选
		if (strcmp(argv[1], "-d") == 0)//更改调试等级
			debugLevel = 1;
		else if (strcmp(argv[1], "-dd") == 0)
			debugLevel = 2;
		strcpy(dnsServerIP, argv[2]);//使用指定的IP地址
		strcpy(filepath, argv[3]);//使用指定的文件路径
		break;
	default:
		printf("Wrong instruction!\n");
	}
}

void dealWithContext(char* recvContext, struct QSF* recvd, int ret)//处理QSF
{
	char tempchar = 0;
	int i = 0, j = 0;
	unsigned short temp;
	//以下开始分析报文内容:
	//拼装域名, e.g. 3www5baidu3com0   ---->   www.baidu.com
	while (i < ret - 12)
	{
		if (recvContext[i] > 0 && recvContext[i] <= 64)//如果是数字
		{
			tempchar = recvContext[i++];
			//i++;
			while (tempchar-- != 0)
				recvd->QNAME[j++] = recvContext[i++];
		}
		if (recvContext[i] != 0) // 不是末尾，则域名加上 "."
			recvd->QNAME[j++] = '.';
		else
		{
			i++; //使recvContext+i指向QTYPE的起始地址
			recvd->QNAME[j] = '\0';
			break;
		}
	}
	memcpy(&temp, recvContext + i, sizeof(unsigned short));
	recvd->QTYPE = ntohs(temp);//将网络字节顺序转化为主机字节顺序
	memcpy(&temp, recvContext + i + 2, sizeof(unsigned short));
	recvd->QCLASS = ntohs(temp);
}

void dealWithHeader(char* recvBuffer, struct HEADER* recvp)//分析报头
{
	unsigned short temp;
	memcpy(&temp, recvBuffer, sizeof(unsigned short));
	recvp->ID = ntohs(temp);
	memcpy(&temp, recvBuffer + 2, sizeof(unsigned short));
	temp = ntohs(temp);
	//recvp->RCODE = (temp & 0x01) + (temp >> 1 & 0x01) * 2 + (temp >> 2 & 0x01) * 4 + (temp >> 3 & 0x01) * 8;
	//recvp->Z = (temp >> 4 & 0x01) + (temp >> 5 & 0x01) * 2 + (temp >> 6 & 0x01) * 4;
	recvp->RCODE = temp & 0x0f;
	recvp->Z = temp >> 4 & 0x07;
	recvp->RA = temp >> 7 & 0x01;
	recvp->RD = temp >> 8 & 0x01;
	recvp->TC = temp >> 9 & 0x01;
	recvp->AA = temp >> 10 & 0x01;
	//recvp->Opcode = (temp >> 11 & 0x01) + (temp >> 12 & 0x01) * 2 + (temp >> 13 & 0x01) * 4 + (temp >> 14 & 0x01) * 8;
	recvp->Opcode = temp >> 11 & 0x0f;
	recvp->QR = temp >> 15 & 0x01;
	memcpy(&temp, recvBuffer + 4, sizeof(unsigned short));
	recvp->QDCOUNT = ntohs(temp);
	memcpy(&temp, recvBuffer + 6, sizeof(unsigned short));
	recvp->ANCOUNT = ntohs(temp);
	memcpy(&temp, recvBuffer + 8, sizeof(unsigned short));
	recvp->NSCOUNT = ntohs(temp);
	memcpy(&temp, recvBuffer + 10, sizeof(unsigned short));
	recvp->ARCOUNT = ntohs(temp);
}

void recordCache(char* recvBuffer, Cache* cache)// 将新响应报文的内容记录到cache文件里
{
	unsigned short qtype;//QTYPE
	char* p = recvBuffer + 12;//跳过报文头，指向Question字段
	char url[100];//ip地址，域名
	struct ipValue* ipval = (struct ipValue*)malloc(sizeof(struct ipValue)), *prePtr;//存储回复字段的IP地址
	ipval->nextval = NULL;
	prePtr = ipval;
	int url_cnt = 0;//url长度
	int nquery = ntohs(*(unsigned short*)(recvBuffer + 4));//问题个数
	int nresponse = ntohs(*(unsigned short*)(recvBuffer + 6));//应答个数
	int ip1, ip2, ip3, ip4;
	for (int i = 0; i < nquery; i++)//默认nquery=1
	{
		while (*p > 0 && *p <= 64)
		{
			int j = *p;
			while (j--) 
				url[url_cnt++] = *(++p);
			url[url_cnt++] = (*(++p) != 0) ? '.' : '\0';
		}
		memcpy(&qtype, p + 1, sizeof(unsigned short));
		qtype = ntohs(qtype);//接收报文的QTYPE字段
		if (qtype != 1)//如果不是IPv4类型，直接退出
			return;
		p += 5;//指向下一个Question字段
	}
	//此时p指针指向应答字段
	if (nresponse > 0 && debugLevel > 1)
		printf("Receive outside %s\n", url);
	int ca_cnt = 0;//回答字段中ipv4的个数
	for (int i = 0; i < nresponse; i++)
	{
		//分析回复报文，默认nrespnse=1
		if ((unsigned char)*p == 0xc0)//是指针
			p += 2;//跳过
		else // 跳过域名
		{
			while (*p > 0)
				p += *p + 1;
			p += 1;
		}
		unsigned short resp_type = ntohs(*(unsigned short*)p);  //回复类型
		if (resp_type == 1)//ipv4
		{
			p += 10;//指针指向RDATA部分，默认是IPv4
			ca_cnt++;
			struct ipValue* curPtr = (struct ipValue*)malloc(sizeof(struct ipValue));
			curPtr->nextval = NULL;
			memset(curPtr->value, 0, sizeof(curPtr->value));
			//memset(temp1->ip, 0, sizeof(temp1->ip)); // 为啥memset
			//读取4个ip部分
			ip1 = (unsigned char)*p++;
			ip2 = (unsigned char)*p++;
			ip3 = (unsigned char)*p++;
			ip4 = (unsigned char)*p++;

			sprintf(curPtr->value, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);//将字符串格式化输出到ip字符串中
			prePtr->nextval = curPtr;
			prePtr = curPtr;
			if (debugLevel >= 2)
				printf("ip %d.%d.%d.%d\n", ip1, ip2, ip3, ip4);
		}
		else // 不是ipv4的情况，则检查长度并跳过
		{
			p += 8;
			int datalen = ntohs(*(unsigned short*)p);   //后面数据长度
			p += 2;//指向后面的数据
			p += datalen;//指向下一个回答字段
		}
	}

	if (ca_cnt == 0)//说明回复报文中没有ipv4地址
		return;

	//LRU
	CacheSet(cache, url, ipval);
	freeValue(ipval);
}