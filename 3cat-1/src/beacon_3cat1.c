/* CubeCAT ******************************************************************************************
*  File:   beacon.c                                                                                *
*  Descr.: Beacon Data generator source.                                                           *
*  Author: Carles Araguz López.                                                                    *
*  Date:   2016-sep-08                                                                             *
*  Vers.:  0.1                                                                                     *
*                                                                                                  *
*  This file is part of the CubeCAT v1.0 project. "CubeCAT" is an educational project developed at *
*  the Technical University of Catalonia - BarcelonaTech (Universitat Politècnica de Catalunya).   *
****************************************************************************************************
*  Changelog:                                                                                      *
*  - v0.1  Araguz C.   Creation.                                                                   *
*  - v0.2  Munoz J.    Adapted for out-of-Skywalker tree                                           *
****************************************************************************************************/
#include "beacon_3cat1.h"

int parse_beacon(unsigned char *buf, size_t size, unsigned char *beacon_type, time_t *sct, unsigned char *soc, unsigned char *sensor_id, unsigned int *ts, float *vs)
{
    time_t sctime;
    unsigned char current_soc, bid, sid;
    float values[255];
    unsigned int times[255];
    int offset, delta_time, delta_min_delay, elems = 0, elem_start, elem_end, ival;
    struct tm *sctime_tm;
    char sctime_str[50];

    /*  Generic beacon packet format:
     *   0        1     2          6      7
     *  ┌────────┬─────┬──────────┬─────┬────────┬───────┬────────┐
     *  │ Header │ Id. │ S/C time │ SoC │ Data 0 │  ...  │ Data N │
     *  └────────┴─────┴──────────┴─────┴────────┴───────┴────────┘
     *
     *  Header (1 byte) = 0x00.
     *  Beacon identifier (1 byte):
     *      0xB0 -> Current state.
     *      0xB1 -> Voltage evolution over time.
     *      0xB2 -> Current evolution over time.
     *      0xB3 -> Temperature evolution over time.
     *      0xB4 -> Irradiance evolution over time.
     *      0xB5 -> State-of-charge evolution over time.
     *  Spacecraft time (4 bytes) = The current UNIX timestamp of the on-board computer.
     *  State-of-charge (1 byte) = 0..100 (or 255 if errored.)
     *
     *  Data (3 or 6 bytes):
     *      Housekeeping values are stored together with a timestamp. Their timestamps, however, are
     *      encoded as a delta time instead of an absolute time. Delta times represent the
     *      difference, in seconds, between the previous housekeeping value and the following one
     *      minus 299 seconds (i.e. T2 = T1 + Δt - 1 + 300). Generally, these delta times can be
     *      encoded in a single byte. In these cases, the format will be as follows:
     *       i         i+1       i+2
     *      ┌─────────┬─────────┬─────────┐
     *      │ Δt      │ Value_1 │ Value_0 │
     *      └─────────┴─────────┴─────────┘
     *      Where delta (Δt) is 1 byte and the value is encoded as a 2-byte unsigned integer. The
     *      value of delta in this case is always greater or equal to 1. If the delta time cannot be
     *      represented in a single byte, then 6 bytes will be used. The format for this case is as
     *      follows:
     *       i+0       i+1       i+2       i+3       i+4       i+5
     *      ┌─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
     *      │ 0x00    │ Δt_2    │ Δt_1    │ Δt_0    │ Value_1 │ Value_0 │
     *      └─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘
     *      In this case, the first byte is set to 0x00 to specify that the delta time is
     *      represented with 3 bytes (Δt_[2..0]).
     */
    bid = buf[1];
    if(buf[0] == 0 && bid >= 0xB0 && bid <= 0xB5) {
        /* This is a valid beacon packet: */

        sctime  = buf[2] << 24;
        sctime += buf[3] << 16;
        sctime += buf[4] << 8;
        sctime += buf[5];
        sctime_tm = localtime(&sctime);
        strftime(sctime_str, 21, "%Y-%m-%d %H:%M:%S", sctime_tm);

        current_soc = buf[6];
        sid = buf[7];

        offset = 8;
        /* Voltage, current, temperature, irradiance and state-of-charge evolution data: */
        while((size_t)offset < size) {
            if(offset == 8 || bid == BEACON_TYPE_STATE) {
                delta_min_delay = 0;
            } else {
                delta_min_delay = BEACON_SENSOR_DELTA_T;
            }
            if(buf[offset] != 0) {
                /* 1-byte Δt. */
                delta_time = buf[offset] - 1 + delta_min_delay;
                if(elems == 0 || bid == BEACON_TYPE_STATE) {
                    times[elems] = sctime - delta_time;
                } else {
                    times[elems] = times[elems - 1] - delta_time;
                }
                ival = (buf[offset + 1] << 8) + buf[offset + 2];
                offset += 3;
            } else {
                /* 3-byte Δt. */
                delta_time  = buf[offset + 1] << 16;
                delta_time += buf[offset + 2] << 8;
                delta_time += buf[offset + 3];
                if(delta_time == 0) {
                    /* End of stream. */
                    break;
                }
                delta_time += delta_min_delay - 1;
                if(elems == 0 || bid == BEACON_TYPE_STATE) {
                    times[elems] = sctime - delta_time;
                } else {
                    times[elems] = times[elems - 1] - delta_time;
                }
                ival = (buf[offset + 4] << 8) + buf[offset + 5];
                offset += 6;
            }
            switch(bid) {
                case BEACON_TYPE_STATE:
                    elem_start = 0;
                    elem_end   = VOLT_SENSORS;
                    if(elems >= elem_start && elems < elem_end) {
                        values[elems] = (float)ival / 1000.0;
                    }
                    elem_start = elem_end;
                    elem_end  += CUR_SENSORS;
                    if(elems >= elem_start && elems < elem_end) {
                        values[elems] = (float)ival;
                    }
                    elem_start = elem_end;
                    elem_end  += TEMP_SENSORS;
                    if(elems >= elem_start && elems < elem_end) {
                        values[elems] = (float)ival;
                    }
                    elem_start = elem_end;
                    elem_end  += IRR_SENSORS;
                    if(elems >= elem_start && elems < elem_end) {
                        values[elems] = (float)ival / 1000.0;
                    }
                    break;
                case BEACON_TYPE_VOLT:
                case BEACON_TYPE_IRR:
                    values[elems] = (float)ival / 1000.0;
                    break;
                case BEACON_TYPE_CURR:
                case BEACON_TYPE_TEMP:
                case BEACON_TYPE_SOC:
                default:
                    values[elems] = (float)ival;
                    break;
            }
            elems++;
        }
        if(ts != NULL) {
            memcpy(ts, times, sizeof(*times) * elems);
        }
        if(vs != NULL) {
            memcpy(vs, values, sizeof(*values) * elems);
        }
        if(sct != NULL) {
            *sct = sctime;
        }
        if(soc != NULL) {
            *soc = current_soc;
        }
        if(beacon_type != NULL) {
            *beacon_type = bid;
        }
        if(sensor_id != NULL) {
            *sensor_id = sid;
        }
    } else {
        elems = EUNDEF;
    }
    return elems;
}

const char * beacon_type_str(unsigned char id)
{
    static char str[25];
    switch(id) {
        case BEACON_TYPE_STATE: strcpy(str, "BEACON_TYPE_STATE");   break;
        case BEACON_TYPE_VOLT:  strcpy(str, "BEACON_TYPE_VOLT");    break;
        case BEACON_TYPE_CURR:  strcpy(str, "BEACON_TYPE_CURR");    break;
        case BEACON_TYPE_TEMP:  strcpy(str, "BEACON_TYPE_TEMP");    break;
        case BEACON_TYPE_IRR:   strcpy(str, "BEACON_TYPE_IRR");     break;
        case BEACON_TYPE_SOC:   strcpy(str, "BEACON_TYPE_SOC");     break;
        default: strcpy(str, "UNKNOWN"); break;
    }
    return str;
}

static bool big_endian(void)
{
    unsigned int i = 1;
    char *c = (char *)&i;
    if(*c) {
        return false;
    } else {
        return true;
    }
}

static void *memcpy_be(void *dst, const void *src, size_t n)
{
    size_t i;
    unsigned char *src0 = (unsigned char *)src;
    unsigned char *dst0 = (unsigned char *)dst;
    if(big_endian()) {
        for(i = 0; i < n; i++) {
            *(dst0 + i) = *(src0 + i);
        }
    } else {
        /* Convert to big endian: */
        for(i = 0; i < n; i++) {
            *(dst0 + i) = *(src0 + n - 1 - i);
        }
    }
    return dst;
}
