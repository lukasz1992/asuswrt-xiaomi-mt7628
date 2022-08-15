#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <httpd.h>
#include <shared.h>

asus_token_t *curr = NULL, *head = NULL;
char last_fail_token[32] = {0};

struct mime_referer mime_referers[] = {
	{ "start_apply.htm", CHECK_REFERER},
	{ "start_apply2.htm", CHECK_REFERER},
	{ "api.asp", CHECK_REFERER},
	{ "applyapp.cgi", CHECK_REFERER},
	{ "apply.cgi", CHECK_REFERER},
	{ "appGet.cgi", CHECK_REFERER},
	{ "upgrade.cgi", CHECK_REFERER},
	{ "upload.cgi", CHECK_REFERER},
#ifdef RTCONFIG_DSL
	{ "dsllog.cgi", CHECK_REFERER},
#endif
	{ "update.cgi", CHECK_REFERER},
#ifdef RTCONFIG_OPENVPN
	{ "vpnupload.cgi", CHECK_REFERER},
#endif
#ifdef RTCONFIG_FINDASUS
	{ "findasus.cgi", CHECK_REFERER},
#endif
	{ "ftpServerTree.cgi", CHECK_REFERER},
#ifdef RTCONFIG_USB
	{ "aidisk/create_account.asp", CHECK_REFERER},
#endif
	{ "status.asp", CHECK_REFERER},
	{ "wds_aplist_2g.asp", CHECK_REFERER},
	{ "wds_aplist_5g.asp", CHECK_REFERER},
	{ "update_networkmapd.asp", CHECK_REFERER},
	{ "get_real_ip.asp", CHECK_REFERER},
	{ "WPS_info.xml", CHECK_REFERER},
	{ "login.cgi", CHECK_REFERER},
#if defined(RTCONFIG_CLOUDSYNC)
	{ "get_webdavInfo.asp", CHECK_REFERER},
#endif
	{ "update_clients.asp", CHECK_REFERER},
	{ NULL, 0 }
};

struct useful_redirect_list useful_redirect_lists[] = {
	{ "Main_**.asp", NULL},
#if defined(RTCONFIG_BWDPI) || defined(RTCONFIG_ROG) || defined(RTCONFIG_TRAFFIC_LIMITER)
	{ "AdaptiveQoS_**.asp", NULL},
#endif
	{ "Advanced_**.asp", NULL},
#if defined(RTCONFIG_CLOUDSYNC)
	{ "aicloud_qis.asp", NULL},
#endif
#if defined(RTCONFIG_USB)
	{ "aidisk.asp", NULL},
#endif
#if defined(RTCONFIG_BWDPI)
	{ "AiProtection_**.asp", NULL},
#endif
#if defined(RTCONFIG_USB)
	{ "APP_Installation.asp", NULL},
#endif
#if defined(RTCONFIG_CLOUDSYNC)
	{ "cloud_**.asp", NULL},
#endif
	{ "Feedback_Info.asp", NULL},
#if defined(RTCONFIG_WTFAST)
	{ "GameBoost.asp", NULL},
#endif
	{ "Guest_network**.asp", NULL},
	{ "index.asp", NULL},
	{ "ParentalControl.asp", NULL},
#if defined(RTCONFIG_USB)
	{ "PrinterServer.asp", NULL},
#endif
	{ "QIS_wizard**.htm", NULL},
	{ "QoS_EZQoS.asp", NULL},
#if !defined(RTCONFIG_BWDPI)
	{ "TrafficAnalyzer_Statistic.asp", NULL},
#endif
	{ "updateSubnet.htm", NULL},
	{ NULL, NULL}
};

int check_model_name(void)
{
	return 1;
}

int get_lang_num(void)
{
	return 9999;
}

int check_lang_support(char *lang)
{
	return 1;
}

int ej_generate_region(int eid, webs_t wp, int argc, char_t **argv)
{
	return 0;
}

int change_preferred_lang()
{
	return (!auto_set_lang && is_firsttime());
}

int useful_redirect_page(char *next_page)
{
	char *buf = NULL;
	char url_str[256];
	struct useful_redirect_list *uptr = NULL;

	strlcpy(url_str, next_page, sizeof(url_str));
	buf = strpbrk(url_str, "?#");
	if(buf)
		*buf = 0x0;
	for(uptr = &useful_redirect_lists[0]; uptr->pattern; ++uptr)
	{
		if(match(uptr->pattern, url_str))
			return 1;
	}
	return 0;
}

char* ipisdomain(char* hostname, char* str)
{
	if (hostname && !strcmp(nvram_safe_get("lan_ipaddr"), hostname))
		strlcpy(str, "chdom()", 8);
	else
		strlcpy(str, "", 1);
	return NULL;
}

char *generate_token(char *token_buf, size_t length)
{
	FILE *fp;
	int i;
	char c;

	if((fp = fopen("/dev/urandom", "r")))
	{
		for(i = 0; (length - 1) > i; ++i)
		{
			if(fread(&c, 1, 1, fp) != 1)
			{
				_dprintf("fread error\n");
				fclose(fp);
				return NULL;
			}
			if(isdigit(c) != 0 || isupper(c) != 0 || islower(c) != 0)
				token_buf[i] = c;
			else
				--i;
		}
		fclose(fp);
		token_buf[length - 1] = 0;
		strlcpy(gen_token, token_buf, length);
		return token_buf;
	}
	_dprintf("fp == NULL\n");
	return NULL;
}

int check_noauth_referrer(char* referer, int fromapp_flag)
{
	char *auth_referer=NULL;

	if(fromapp_flag != 0)
		return 0;

	if(!referer || !strlen(host_name)){
		return NOREFERER;
	}else{
		auth_referer = get_referrer(referer);
	}

	if(!strcmp(host_name, auth_referer))
		return 0;
	else
		return REFERERFAIL;
}

int referer_check(char* referer, int fromapp_flag)
{
	char *auth_referer=NULL;
	const int d_len = strlen(DUT_DOMAIN_NAME);
	int port = 0;
	int referer_from_https = 0;
	int referer_host_check = 0;

	if(fromapp_flag != 0)
		return 0;
	if(!referer){
		return NOREFERER;
	}else{
		auth_referer = get_referrer(referer);
	}

	if(referer_host[0] == 0){
		return WEB_NOREFERER;
	}

	if(!strcmp(host_name, auth_referer)) referer_host_check = 1;

	if (*(auth_referer + d_len) == ':' && (port = atoi(auth_referer + d_len + 1)) > 0 && port < 65536)
		referer_from_https = 1;

	if (((strlen(auth_referer) == d_len) || (*(auth_referer + d_len) == ':' && atoi(auth_referer + d_len + 1) > 0))
	   && strncmp(DUT_DOMAIN_NAME, auth_referer, d_len)==0){
		if(referer_from_https)
			snprintf(auth_referer,sizeof(referer_host),"%s:%d",nvram_safe_get("lan_ipaddr"), port);
		else
			snprintf(auth_referer,sizeof(referer_host),"%s",nvram_safe_get("lan_ipaddr"));
	}

	/* form based referer info? */
	if(referer_host_check && (strlen(auth_referer) == strlen(referer_host)) && strncmp( auth_referer, referer_host, strlen(referer_host) ) == 0){
		//_dprintf("asus token referer_check: the right user and password\n");
		return 0;
	}else{
		//_dprintf("asus token referer_check: the wrong user and password\n");
		return REFERERFAIL;
	}
	return REFERERFAIL;
}

asus_token_t *search_token_in_list(char *token, asus_token_t **prev)
{
	asus_token_t *tkptr = head;
	asus_token_t *tmp = NULL;
	char *login_ip_str;
	struct in_addr login_ip_addr;

	login_ip_addr.s_addr = login_ip_tmp;
	login_ip_str = inet_ntoa(login_ip_addr);

	while(tkptr != NULL)
	{
		if((!strncmp(token, tkptr->token, sizeof(tkptr->token)) && nvram_match("x_Setting", "0")) || (!strcmp(user_agent, tkptr->useragent) && !strcmp(login_ip_str, tkptr->ipaddr)))
		{
			if(prev)
				*prev = tmp;
			return tkptr;
		}
		tmp = tkptr;
		tkptr = tkptr->next;
	}
	return NULL;
}

int delete_logout_from_list(char *cookies)
{
	char *location_cp;
	char *cur;
	asus_token_t *tkptr = NULL;
	asus_token_t *prev = NULL;
	char asustoken[32];

	memset(asustoken, 0, sizeof(asustoken));
	if(!cookies || nvram_match("x_Setting", "0"))
		return 0;

	if((location_cp = strstr(cookies, "asus_token")) == NULL)
		return 0;
	cur = &location_cp[11];
	cur += strspn(cur, " \t");
	snprintf(asustoken, sizeof(asustoken), "%s", cur);
	if((tkptr = search_token_in_list(asustoken, &prev)) == NULL)
		return -1;
	if(prev != NULL)
		prev->next = tkptr->next;
	if(tkptr == curr)
		curr = prev;
	if(tkptr == head)
		head = tkptr->next;
	free(tkptr);
	tkptr = NULL;
	return 0;
}

inline int get_token_list_length(void)
{
	int count = 0;
	asus_token_t *tkptr = head;

	while(tkptr != NULL){
		count++;
		tkptr = tkptr->next;
	}
	return count;
}

inline asus_token_t* search_timeout_in_list(asus_token_t **prev, int fromapp_flag)
{
	asus_token_t *tkptr = head;
	asus_token_t *tmp = NULL;
	int logout_time = 30, found = 0;
	time_t now = 0;

	if(!nvram_match("http_autologout", "0"))
		logout_time = nvram_get_int("http_autologout");
	now = uptime();
	while(tkptr != NULL){
		if(now - atol(tkptr->login_timestampstr) > (60 * logout_time) && check_user_agent(tkptr->useragent) == 0){
			found = 1;
			break;
		}else if((now - atol(tkptr->login_timestampstr)) > 6000 && check_user_agent(tkptr->useragent) != 0 && check_user_agent(tkptr->useragent) != FROM_IFTTT){
			found = 1;
			break;
		}else if(fromapp_flag == 0 && check_user_agent(tkptr->useragent) == 0){
			found = 1;
			break;
		}else{
			tmp = tkptr;
			tkptr = tkptr->next;
		}
	}
	if(found == 1){
		if(prev)
			*prev = tmp;
		return tkptr;
	}
	return NULL;
}

int check_token_timeout_in_list(void)
{
	int i;
	int list_len = get_token_list_length();
	int fromapp_flag = check_user_agent(user_agent);
	asus_token_t *del = NULL;
	asus_token_t *prev = NULL;

	for(i = 0; i < list_len; i++)
	{
		del = search_timeout_in_list(&prev, fromapp_flag);
		if(del == NULL)
			return -1;
		if(prev != NULL)
			prev->next = del->next;
		if(del == curr)
			curr = prev;
		if(del == head)
			head = del->next;
		free(del);
		del = NULL;
	}
	return 0;
}

inline asus_token_t* add_token_to_list(char *token, int add_to_end)
{
	asus_token_t *tkptr = NULL;
	char login_timestr[32];
	char *login_ip_str;
	struct in_addr login_ip_addr;
	time_t now;

	tkptr = (asus_token_t *)malloc(sizeof(asus_token_t));
	if(tkptr == NULL)
	{
		_dprintf("\n Node creation failed \n");
		return NULL;
	}

	login_ip_addr.s_addr = login_ip_tmp;
	login_ip_str = inet_ntoa(login_ip_addr);
	now = uptime();

	memset(login_timestr, 0, sizeof(login_timestr));
	snprintf(login_timestr, sizeof(login_timestr), "%lu", now);
	strncpy(tkptr->useragent, user_agent, sizeof(tkptr->useragent));
	strncpy(tkptr->token, token, sizeof(tkptr->token));
	strncpy(tkptr->ipaddr, login_ip_str, sizeof(tkptr->ipaddr));
	strncpy(tkptr->login_timestampstr, login_timestr, sizeof(tkptr->login_timestampstr));
	strncpy(tkptr->host, host_name, sizeof(tkptr->host));
	tkptr->next = NULL;
	if (!head)
		head = tkptr;
	else
		curr->next = tkptr;
	curr = tkptr;
	return tkptr;
}

void add_asus_token(char *token) {
	check_token_timeout_in_list();
	add_token_to_list(token, 1);
}

#define	HEAD_HTTP_LOGIN	"HTTP login"	// copy from push_log/push_log.h

int auth_check(char* dirname, char* authorization, char* url, char* file, char* cookies, int fromapp_flag)
{
	struct in_addr temp_ip_addr;
	char *temp_ip_str;
	time_t dt;
	char asustoken[32];
	char *cp=NULL, *location_cp;

	memset(asustoken,0,sizeof(asustoken));

	login_timestamp_tmp = uptime();
	dt = login_timestamp_tmp - last_login_timestamp;
	if(last_login_timestamp != 0 && dt > 60){
		login_try = 0;
		last_login_timestamp = 0;
		lock_flag = 0;
	}
	if (MAX_login <= DEFAULT_LOGIN_MAX_NUM)
		MAX_login = DEFAULT_LOGIN_MAX_NUM;
	if(login_try >= MAX_login){
		lock_flag = 1;
		temp_ip_addr.s_addr = login_ip_tmp;
		temp_ip_str = inet_ntoa(temp_ip_addr);

		if(login_try%MAX_login == 0){
			check_token_timeout_in_list();
			logmessage(HEAD_HTTP_LOGIN, "Detect abnormal logins at %d times. The newest one was from %s in auth check.", login_try, temp_ip_str);
		}

//#ifdef LOGIN_LOCK
		__send_login_page(fromapp_flag, LOGINLOCK, url, NULL, dt);
		return LOGINLOCK;
//#endif
	}

	/* Is this directory unprotected? */
	if ( !strlen(auth_passwd) ){
		/* Yes, let the request go through. */
		return 0;
	}

	if(!cookies){
		send_login_page(fromapp_flag, NOTOKEN, url, file, 0);
		return NOTOKEN;
	}else{
		location_cp = strstr(cookies,"asus_token");
		if(location_cp != NULL){
			cp = &location_cp[11];
			cp += strspn( cp, " \t" );
			snprintf(asustoken, sizeof(asustoken), "%s", cp);
		}else{
			send_login_page(fromapp_flag, NOTOKEN, url, file, 0);
			return NOTOKEN;
		}
	}
	/* form based authorization info? */

	if(search_token_in_list(asustoken, NULL) != NULL){
		//_dprintf("asus token auth_check: the right user and password\n");
#ifdef RTCONFIG_IFTTT
		if(strncmp(url, GETIFTTTCGI, strlen(GETIFTTTCGI))==0) add_ifttt_flag();
#endif
		login_try = 0;
		last_login_timestamp = 0;
		lock_flag = 0;
		return 0;
	}else{
		//_dprintf("asus token auth_check: the wrong user and password\n");
		if(!strcmp(last_fail_token, asustoken))
			send_login_page(fromapp_flag, AUTHFAIL, url, file, dt);
		else{
			strlcpy(last_fail_token, asustoken, sizeof(last_fail_token));
			__send_login_page(fromapp_flag, AUTHFAIL, url, file, dt);
		}
		return AUTHFAIL;
	}

	send_login_page(fromapp_flag, AUTHFAIL, url, file, 0);
	return AUTHFAIL;
}
