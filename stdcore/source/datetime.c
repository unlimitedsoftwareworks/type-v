#include "datetime.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

// Helper function to determine if a year is a leap year
static bool isLeapYear(uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Helper function to get the number of days in a given month
static int getDaysInMonth(uint16_t year, uint8_t month) {
    static const int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month < 1 || month > 12) {
        return -1;
    }
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return daysInMonth[month - 1];
}

// Create a new DateTime object with the current date and time
dt_t dt_now(bool *valid) {
    dt_t dt;
#ifdef _WIN32
    SYSTEMTIME st;
    GetSystemTime(&st);

    dt.year = st.wYear;
    dt.month = st.wMonth;
    dt.day = st.wDay;
    dt.hour = st.wHour;
    dt.minute = st.wMinute;
    dt.second = st.wSecond;
    dt.millisecond = st.wMilliseconds;
    *valid = true;
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        *valid = false;
        return dt;
    }

    struct tm t;
    if (gmtime_r(&ts.tv_sec, &t) == NULL) {
        *valid = false;
        return dt;
    }

    dt.year = t.tm_year + 1900;
    dt.month = t.tm_mon + 1;
    dt.day = t.tm_mday;
    dt.hour = t.tm_hour;
    dt.minute = t.tm_min;
    dt.second = t.tm_sec;
    dt.millisecond = ts.tv_nsec / 1000000;
    *valid = true;
#endif
    return dt;
}

// Create a new DateTime object from specific components
dt_t dt_create(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond, bool *valid) {
    dt_t dt = { year, month, day, hour, minute, second, millisecond };
    *valid = dt_isValid(&dt);
    return dt;
}

// Validate if a DateTime object has valid values
bool dt_isValid(const dt_t *dt) {
    if (dt->year < 1970 || dt->month < 1 || dt->month > 12) {
        return false;
    }
    int daysInMonth = getDaysInMonth(dt->year, dt->month);
    if (daysInMonth == -1 || dt->day < 1 || dt->day > daysInMonth) {
        return false;
    }
    if (dt->hour > 23 || dt->minute > 59 || dt->second > 59 || dt->millisecond > 999) {
        return false;
    }
    return true;
}

// Parse a date-time string in ISO 8601 format
dt_t dt_parse(const char *isoString, bool *valid) {
    dt_t dt;
    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    int milliseconds = 0;

    if (sscanf(isoString, "%4d-%2d-%2dT%2d:%2d:%2d.%3dZ", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &milliseconds) == 7) {
        t.tm_year -= 1900;
        t.tm_mon -= 1;
        dt.year = t.tm_year + 1900;
        dt.month = t.tm_mon + 1;
        dt.day = t.tm_mday;
        dt.hour = t.tm_hour;
        dt.minute = t.tm_min;
        dt.second = t.tm_sec;
        dt.millisecond = milliseconds;
        *valid = dt_isValid(&dt);
    } else {
        *valid = false;
    }
    return dt;
}

// Get the ISO 8601 formatted string representation of the DateTime object
void dt_toISOString(const dt_t *dt, char *buffer, size_t bufferSize, bool *valid) {
    if (!dt_isValid(dt) || bufferSize < 25) {
        *valid = false;
        return;
    }
    int result = snprintf(buffer, bufferSize, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", dt->year, dt->month, dt->day, dt->hour, dt->minute, dt->second, dt->millisecond);
    *valid = (result > 0 && (size_t)result < bufferSize);
}

// Get the UTC timestamp in milliseconds since Unix epoch
int64_t dt_toUnixTimestamp(const dt_t *dt, bool *valid) {
    if (!dt_isValid(dt)) {
        *valid = false;
        return 0;
    }

    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    t.tm_year = dt->year - 1900;
    t.tm_mon = dt->month - 1;
    t.tm_mday = dt->day;
    t.tm_hour = dt->hour;
    t.tm_min = dt->minute;
    t.tm_sec = dt->second;
#ifdef _WIN32
    time_t seconds = _mkgmtime(&t);
#else
    time_t seconds = timegm(&t);
#endif

    if (seconds == -1) {
        *valid = false;
        return 0;
    }

    *valid = true;
    return (int64_t)seconds * 1000 + dt->millisecond;
}

// Create a DateTime object from a Unix timestamp
dt_t dt_fromUnixTimestamp(int64_t timestamp, bool *valid) {
    dt_t dt;
    time_t seconds = timestamp / 1000;
    int32_t milliseconds = timestamp % 1000;

    struct tm t;
#ifdef _WIN32
    if (gmtime_s(&t, &seconds) != 0) {
        *valid = false;
        return dt;
    }
#else
    if (gmtime_r(&seconds, &t) == NULL) {
        *valid = false;
        return dt;
    }
#endif

    dt.year = t.tm_year + 1900;
    dt.month = t.tm_mon + 1;
    dt.day = t.tm_mday;
    dt.hour = t.tm_hour;
    dt.minute = t.tm_min;
    dt.second = t.tm_sec;
    dt.millisecond = milliseconds;
    *valid = true;
    return dt;
}

// Compare two DateTime objects
int dt_compare(const dt_t *dt1, const dt_t *dt2) {
    int64_t ts1, ts2;
    bool valid;
    ts1 = dt_toUnixTimestamp(dt1, &valid);
    if (!valid) return 0;
    ts2 = dt_toUnixTimestamp(dt2, &valid);
    if (!valid) return 0;
    return (ts1 < ts2) ? -1 : (ts1 > ts2) ? 1 : 0;
}

// Add days, hours, minutes, seconds, and milliseconds to a DateTime object
dt_t dt_add(const dt_t *dt, int32_t days, int32_t hours, int32_t minutes, int32_t seconds, int32_t milliseconds, bool *valid) {
    int64_t timestamp = dt_toUnixTimestamp(dt, valid);
    if (!*valid) return *dt;

    timestamp += ((int64_t)days * 86400000) + ((int64_t)hours * 3600000) + ((int64_t)minutes * 60000) + ((int64_t)seconds * 1000) + milliseconds;
    return dt_fromUnixTimestamp(timestamp, valid);
}

// Subtract days, hours, minutes, seconds, and milliseconds from a DateTime object
dt_t dt_subtract(const dt_t *dt, int32_t days, int32_t hours, int32_t minutes, int32_t seconds, int32_t milliseconds, bool *valid) {
    return dt_add(dt, -days, -hours, -minutes, -seconds, -milliseconds, valid);
}

// Get the day of the week for a DateTime object
int dt_getDayOfWeek(const dt_t *dt) {
    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    t.tm_year = dt->year - 1900;
    t.tm_mon = dt->month - 1;
    t.tm_mday = dt->day;
#ifdef _WIN32
    time_t seconds = _mkgmtime(&t);
#else
    time_t seconds = timegm(&t);
#endif

    if (seconds == -1) {
        return -1;
    }

#ifdef _WIN32
    if (gmtime_s(&t, &seconds) != 0) {
        return -1;
    }
#else
    struct tm *result = gmtime(&seconds);
    if (result == NULL) {
        return -1;
    }
#endif
    return t.tm_wday;
}

// Convert DateTime to local time
dt_t dt_toLocalTime(const dt_t *dt, bool *valid) {
    int64_t timestamp = dt_toUnixTimestamp(dt, valid);
    if (!*valid) return *dt;

    time_t seconds = timestamp / 1000;
    int32_t milliseconds = timestamp % 1000;

    struct tm t;
#ifdef _WIN32
    if (localtime_s(&t, &seconds) != 0) {
        *valid = false;
        return *dt;
    }
#else
    if (localtime_r(&seconds, &t) == NULL) {
        *valid = false;
        return *dt;
    }
#endif

    dt_t local;
    local.year = t.tm_year + 1900;
    local.month = t.tm_mon + 1;
    local.day = t.tm_mday;
    local.hour = t.tm_hour;
    local.minute = t.tm_min;
    local.second = t.tm_sec;
    local.millisecond = milliseconds;
    *valid = true;
    return local;
}

// Convert DateTime to UTC time
dt_t dt_toUTC(const dt_t *dt, bool *valid) {
    return *dt;
}

// Get the year component of the DateTime object
uint16_t dt_getYear(const dt_t *dt) {
    return dt->year;
}

// Get the month component of the DateTime object
uint8_t dt_getMonth(const dt_t *dt) {
    return dt->month;
}


// Get the hour component of the DateTime object
uint8_t dt_getHour(const dt_t *dt) {
    return dt->hour;
}

// Get the minute component of the DateTime object
uint8_t dt_getMinute(const dt_t *dt) {
    return dt->minute;
}

// Get the second component of the DateTime object
uint8_t dt_getSecond(const dt_t *dt) {
    return dt->second;
}

// Get the millisecond component of the DateTime object
uint16_t dt_getMillisecond(const dt_t *dt) {
    return dt->millisecond;
}

// Calculate the difference between two DateTime objects
int64_t dt_diff(const dt_t *dt1, const dt_t *dt2, bool *valid) {
    int64_t ts1 = dt_toUnixTimestamp(dt1, valid);
    if (!*valid) return 0;
    int64_t ts2 = dt_toUnixTimestamp(dt2, valid);
    if (!*valid) return 0;
    return ts1 - ts2;
}

// Format the DateTime object using a custom format string
void dt_format(const dt_t *dt, const char *format, char *buffer, size_t bufferSize, bool *valid) {
    if (!dt_isValid(dt) || bufferSize == 0) {
        *valid = false;
        return;
    }

    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    t.tm_year = dt->year - 1900;
    t.tm_mon = dt->month - 1;
    t.tm_mday = dt->day;
    t.tm_hour = dt->hour;
    t.tm_min = dt->minute;
    t.tm_sec = dt->second;

    size_t result = strftime(buffer, bufferSize, format, &t);
    *valid = (result > 0);
}
