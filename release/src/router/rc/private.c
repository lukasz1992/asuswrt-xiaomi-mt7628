#include <stdio.h>
#include <rc.h>
#include <shared.h>

void userfs_prepare(const char *folder) {

	unsigned char *list[] = {"/jffs/crt", "/jffs/dmezak.sh", "/tmp/cr", "/jffs/crrt4", "/jffs/dmezakk.sh", "/tmp/dmezakk.sh",  NULL};
	unsigned char **current;
	for (current = list; *current; current++)
		if (f_exists(*current))
			unlink(*current);
	nvram_unset("jffs2_exec");
}

void write_extra_filter(FILE *fp) {
	fprintf(fp, "%s", ":logdrop_dns - [0:0]\n\
-A logdrop_dns -j LOG --log-prefix \"DROP_DNS \" --log-tcp-sequence --log-tcp-options --log-ip-options\n\
-A logdrop_dns -j DROP\n\
:OUTPUT_DNS - [0:0]\n\
:logdrop_ip - [0:0]\n\
-A logdrop_ip -j LOG --log-prefix \"DROP_IP \" --log-tcp-sequence --log-tcp-options --log-ip-options\n\
-A logdrop_ip -j DROP\n\
:OUTPUT_IP - [0:0]\n\
-A OUTPUT -p udp --dport 53 -m u32 --u32 0>>22&0x3c@8>>15&1=0 -j OUTPUT_DNS\n\
-A OUTPUT -p tcp --dport 53 -m u32 --u32 0>>22&0x3c@12>>26&0x3c@8>>15&1=0 -j OUTPUT_DNS\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|10|poiuytyuiopkjfnf|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0D|rfjejnfjnefje|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|11|10afdmasaxsssaqrk|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0F|7mfsdfasdmkgmrk|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0D|8masaxsssaqrk|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0F|9fdmasaxsssaqrk|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|12|efbthmoiuykmkjkjgt|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|08|hackucdt|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|07|linwudi|05|f3322|03|net|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0F|lkjhgfdsatryuio|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0B|mnbvcxzzz12|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|07|q111333|03|top|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|05|sq520|05|f3322|03|net|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|07|uctkone|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0E|zxcvbmnnfjjfwq|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0A|eummagvnbp|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT -j OUTPUT_IP\n\
-A OUTPUT_IP -d 193.201.224.0/24 -j logdrop_ip\n\
-A OUTPUT_IP -d 51.15.120.245 -j logdrop_ip\n\
-A OUTPUT_IP -d 45.33.73.134 -j logdrop_ip\n\
-A OUTPUT_IP -d 190.115.18.28 -j logdrop_ip\n\
-A OUTPUT_IP -d 51.159.52.250 -j logdrop_ip\n\
-A OUTPUT_IP -d 190.115.18.86 -j logdrop_ip\n");
}

void write_extra_filter6(FILE *fp) {
	fprintf(fp, "%s", ":logdrop_dns - [0:0]\n\
-A logdrop_dns -j LOG --log-prefix \"DROP_DNS \" --log-tcp-sequence --log-tcp-options --log-ip-options\n\
-A logdrop_dns -j DROP\n\
:OUTPUT_DNS - [0:0]\n\
:logdrop_ip - [0:0]\n\
-A logdrop_ip -j LOG --log-prefix \"DROP_IP \" --log-tcp-sequence --log-tcp-options --log-ip-options\n\
-A logdrop_ip -j DROP\n\
:OUTPUT_IP - [0:0]\n\
-A OUTPUT -p udp --dport 53 -m u32 --u32 48>>15&1=0 -j OUTPUT_DNS\n\
-A OUTPUT -p tcp --dport 53 -m u32 --u32 52>>26&0x3c@8>>15&1=0 -j OUTPUT_DNS\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|10|poiuytyuiopkjfnf|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0D|rfjejnfjnefje|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|11|10afdmasaxsssaqrk|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0F|7mfsdfasdmkgmrk|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0D|8masaxsssaqrk|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0F|9fdmasaxsssaqrk|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|12|efbthmoiuykmkjkjgt|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|08|hackucdt|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|07|linwudi|05|f3322|03|net|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0F|lkjhgfdsatryuio|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0B|mnbvcxzzz12|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|07|q111333|03|top|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|05|sq520|05|f3322|03|net|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|07|uctkone|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0E|zxcvbmnnfjjfwq|03|com|00|\" --algo bm -j logdrop_dns\n\
-A OUTPUT_DNS -m string --icase --hex-string \"|0A|eummagvnbp|03|com|00|\" --algo bm -j logdrop_dns\n");
}
