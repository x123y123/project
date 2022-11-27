void *gps(void *param);
int hex2int(char *c);
int checksum_valid(char *string);
int parse_comma_delimited_str(char *string, char **fields, int max_fields);
int debug_print_fields(int numfields, char **fields);
int OpenGPSPort(const char *devname);
int SetTime(char *date, char *time);

