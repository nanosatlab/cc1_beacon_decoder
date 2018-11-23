#include "beacon_decoder.h"

MYSQL *con;
socket_handler_t s = { .fd = 0};

void print_beacon_data(unsigned char *buf, size_t size)
{
    unsigned char bid, soc, sid;
    time_t sct;
    unsigned int ts[MAX_BEACON_DATA];
    float vs[MAX_BEACON_DATA];
    int nbdata, i;
    struct tm *time_tm;
    char time_str[25], str_color[10];
    bool time_critical = false, soc_critical = false;
    char mysql_query_str[1024];
    printf("Beacon data received. Printing its contents now:\n");
    nbdata = parse_beacon(buf, size, &bid, &sct, &soc, &sid, ts, vs);
    if(nbdata > 0 && bid >= BEACON_TYPE_STATE && bid <= BEACON_TYPE_SOC) {
        if(bid == BEACON_TYPE_STATE) {
            // TODO
        } else {
            time_critical = abs(sct - time(NULL)) >= 600;
            soc_critical = soc < 40 || soc > 100;
            if(time_critical || soc_critical) {
                sprintf(str_color, "\x1b[31;1m");
            } else {
                memset(str_color, 0, 10);
            }

            time_tm = localtime(&sct);
            strftime(time_str, 25, "%Y-%m-%d %H:%M:%S", time_tm);

            printf("%s-- Beacon ID: %s [%d housekeeping registers]\x1b[0m\n", str_color, beacon_type_str(bid), nbdata);
            printf("%s%s Space-segment time: %s. Timestamp: %ld, skew: %ld\x1b[0m\n",
                str_color,
                (time_critical ? "(!)" : "--"),
                time_str, sct, (sct - time(NULL))
            );
            printf("%s%s State-of-charge: %d %%\x1b[0m\n", str_color, (soc_critical ? "(!)" : "--"), soc);
            printf("%s-- Sensor ID: %d \x1b[0m\n", str_color, sid);
        }
    } else {
        printf("Beacon packet had no data or couldn't be parsed [%d registers; BID:0x%.X %s]\n", nbdata, bid, beacon_type_str(bid));
    }
}

void save_beacon_data(unsigned char *buf, size_t size)
{
    unsigned char bid, soc, sid;
    time_t sct;
    unsigned int ts[MAX_BEACON_DATA];
    float vs[MAX_BEACON_DATA];
    int nbdata, i;
    struct tm *time_tm;
    char time_str[25], str_color[10];
    bool time_critical = false, soc_critical = false;
    char mysql_query_str[1024];
    printf("Beacon data received. Printing its contents now:\n");
    nbdata = parse_beacon(buf, size, &bid, &sct, &soc, &sid, ts, vs);
    if(nbdata > 0 && bid >= BEACON_TYPE_STATE && bid <= BEACON_TYPE_SOC) {
        if(bid == BEACON_TYPE_STATE) {
            // TODO
        } else {
            /* insert into the current DB the SCT + SOC */
            sprintf(mysql_query_str, "INSERT INTO soc VALUES (%ld,%d,%d)", sct, 0, soc);
            mysql_query(con, mysql_query_str);
            /* perform a save into the DB */
            switch(bid) {
                case BEACON_TYPE_STATE:

                    break;
                case BEACON_TYPE_VOLT:
                    for(i = 0; i < nbdata; i++) {
                        sprintf(mysql_query_str, "INSERT INTO voltage VALUES (%ld,%d,%f)", (time_t) ts[i], sid, vs[i]);
                        mysql_query(con, mysql_query_str);
                    }
                    break;
                case BEACON_TYPE_CURR:
                    for(i = 0; i < nbdata; i++) {
                        sprintf(mysql_query_str, "INSERT INTO current VALUES (%ld,%d,%f)", (time_t) ts[i], sid, vs[i]);
                        mysql_query(con, mysql_query_str);
                    }
                    break;
                case BEACON_TYPE_TEMP:
                    for(i = 0; i < nbdata; i++) {
                        sprintf(mysql_query_str, "INSERT INTO temperature VALUES (%ld,%d,%f)", (time_t) ts[i], sid, vs[i]);
                        mysql_query(con, mysql_query_str);
                    }
                    break;
                case BEACON_TYPE_IRR:
                    for(i = 0; i < nbdata; i++) {
                        sprintf(mysql_query_str, "INSERT INTO irradiance VALUES (%ld,%d,%f)", (time_t) ts[i], sid, vs[i]);
                        mysql_query(con, mysql_query_str);
                    }
                    break;
                case BEACON_TYPE_SOC:
                    for(i = 0; i < nbdata; i++) {
                        sprintf(mysql_query_str, "INSERT INTO soc VALUES (%ld,%d,%d)", (time_t) ts[i], sid, (int) vs[i]);
                        mysql_query(con, mysql_query_str);
                    }
                    break;
                default:
                    break;
            }
        }
    } else {
        printf("Beacon packet had no data or couldn't be parsed [%d registers; BID:0x%.X %s]\n", nbdata, bid, beacon_type_str(bid));
    }
}

int beacon_receive_packet(int spifd, unsigned char *data, int *len)
{
    /* fd tcp server at localhost:52001 */
    /* make a connect, receive and close */
    (void) (spifd);
    int ret;
    int func_ret = ETIMEOUT;
    socket_config_t conf;

    strcpy(conf.client.ip, M_USRP_HOST_IP);
    conf.client.port = 52000;
    if(s.fd == 0) {
        ret = client_socket_init(&conf, &s);
        if(ret == SU_IO_ERROR) {
            printf("Error trying to connect to socket IP: %s for receive_packet\n", M_USRP_HOST_IP);
            close(s.fd);
            s.fd = 0;
            sleep(1);
        }
    }
    if(s.fd != 0) {
        s.expected_len = 223;
        s.timeout_ms = 2000;
        func_ret = ETIMEOUT;
        if((ret = socket_read(&s)) == SU_NO_ERROR) {
            func_ret = NOERROR;
            *len = s.len;
            memcpy(data, s.buffer, s.len);
        } else if (ret == SU_IO_ERROR) {
            printf("Error trying to connect to socket IP: %s for receive_packet\n", M_USRP_HOST_IP);
            close(s.fd);
            s.fd = 0;
            sleep(1);
        }
    }
    /* Access functions */
    return func_ret;
}

int main(void)
{
    unsigned char beacon_packet[223];
    int len;
    con = mysql_init(NULL);
    if(con == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }
    if (mysql_real_connect(con, "localhost", "goperator", "cubecat", "gsdb", 0, NULL, 0) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        exit(1);
    }
    while(1) {
        if(beacon_receive_packet(0, beacon_packet, &len) == NOERROR) {
            save_beacon_data(beacon_packet, len);
            print_beacon_data(beacon_packet, len);
        }
    }
    mysql_close(con);
    exit(0);
}
