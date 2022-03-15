struct tcode_lang_s {
	int model;
	char *odmpid;
	char *tcode;
	char *support_lang;
	int auto_change;
};

const struct tcode_lang_s tcode_lang_list[] = {
	{ -1, "", "GLOBAL", "BR CN CZ DE EN ES FR HU IT JP KR MS NL PL RU RO SL TH TR TW UK DA FI NO SV", 1 },
	{ 0, 0, 0, 0, 0 },
};
