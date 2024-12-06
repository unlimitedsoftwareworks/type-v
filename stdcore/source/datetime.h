#ifndef DATETIME_H
#define DATETIME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Define the DateTime structure
/**
 * @brief Represents a date and time with millisecond precision.
 */
typedef struct {
    uint16_t year;        // Sufficient for a very large range of years
    uint8_t month;       // 1-12
    uint8_t day;         // 1-31
    uint8_t hour;        // 0-23
    uint8_t minute;      // 0-59
    uint8_t second;      // 0-59
    uint16_t millisecond; // 0-999
} dt_t;

/**
 * @brief Create a new DateTime object with the current date and time.
 *
 * @param[out] valid Pointer to a boolean indicating if the creation was successful.
 * @return dt_t object representing the current date and time.
 */
dt_t dt_now(bool *valid);

/**
 * @brief Create a new DateTime object from specific components.
 *
 * @param year Year component.
 * @param month Month component (1-12).
 * @param day Day component (1-31).
 * @param hour Hour component (0-23).
 * @param minute Minute component (0-59).
 * @param second Second component (0-59).
 * @param millisecond Millisecond component (0-999).
 * @param[out] valid Pointer to a boolean indicating if the creation was successful.
 * @return dt_t object representing the specified date and time.
 */
dt_t dt_create(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond, bool *valid);

/**
 * @brief Parse a date-time string in ISO 8601 format.
 *
 * @param isoString String in ISO 8601 format (e.g., "2024-12-05T14:30:00.000Z").
 * @param[out] valid Pointer to a boolean indicating if parsing was successful.
 * @return dt_t object parsed from the string.
 */
dt_t dt_parse(const char *isoString, bool *valid);

/**
 * @brief Get the ISO 8601 formatted string representation of the DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @param buffer Buffer to store the formatted string.
 * @param bufferSize Size of the buffer.
 * @param[out] valid Pointer to a boolean indicating if formatting was successful.
 */
void dt_toISOString(const dt_t *dt, char *buffer, size_t bufferSize, bool *valid);

/**
 * @brief Get the UTC timestamp in milliseconds since Unix epoch.
 *
 * @param dt Pointer to the dt_t object.
 * @param[out] valid Pointer to a boolean indicating if conversion was successful.
 * @return UTC timestamp in milliseconds since Unix epoch.
 */
int64_t dt_toUnixTimestamp(const dt_t *dt, bool *valid);

/**
 * @brief Create a DateTime object from a Unix timestamp.
 *
 * @param timestamp Unix timestamp in milliseconds since Unix epoch.
 * @param[out] valid Pointer to a boolean indicating if creation was successful.
 * @return dt_t object representing the specified timestamp.
 */
dt_t dt_fromUnixTimestamp(int64_t timestamp, bool *valid);

/**
 * @brief Add days, hours, minutes, seconds, and milliseconds to a DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @param days Number of days to add.
 * @param hours Number of hours to add.
 * @param minutes Number of minutes to add.
 * @param seconds Number of seconds to add.
 * @param milliseconds Number of milliseconds to add.
 * @param[out] valid Pointer to a boolean indicating if the addition was successful.
 * @return dt_t object with the added components.
 */
dt_t dt_add(const dt_t *dt, int32_t days, int32_t hours, int32_t minutes, int32_t seconds, int32_t milliseconds, bool *valid);

/**
 * @brief Subtract days, hours, minutes, seconds, and milliseconds from a DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @param days Number of days to subtract.
 * @param hours Number of hours to subtract.
 * @param minutes Number of minutes to subtract.
 * @param seconds Number of seconds to subtract.
 * @param milliseconds Number of milliseconds to subtract.
 * @param[out] valid Pointer to a boolean indicating if the subtraction was successful.
 * @return dt_t object with the subtracted components.
 */
dt_t dt_subtract(const dt_t *dt, int32_t days, int32_t hours, int32_t minutes, int32_t seconds, int32_t milliseconds, bool *valid);

/**
 * @brief Compare two DateTime objects.
 *
 * @param dt1 Pointer to the first dt_t object.
 * @param dt2 Pointer to the second dt_t object.
 * @return Negative value if dt1 is earlier than dt2, positive value if dt1 is later than dt2, and zero if they are equal.
 */
int dt_compare(const dt_t *dt1, const dt_t *dt2);

/**
 * @brief Check if a year is a leap year.
 *
 * @param year Year to check.
 * @return True if the year is a leap year, false otherwise.
 */
bool dt_isLeapYear(uint16_t year);

/**
 * @brief Get the day of the week for a DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @return Day of the week (0 = Sunday, 6 = Saturday).
 */
int dt_getDayOfWeek(const dt_t *dt);

/**
 * @brief Get the number of days in a given month of a specific year.
 *
 * @param year Year component.
 * @param month Month component (1-12).
 * @return Number of days in the specified month.
 */
int dt_getDaysInMonth(uint16_t year, uint8_t month);

/**
 * @brief Validate if a DateTime object has valid values.
 *
 * @param dt Pointer to the dt_t object.
 * @return True if the dt_t object is valid, false otherwise.
 */
bool dt_isValid(const dt_t *dt);

/**
 * @brief Convert DateTime to local time.
 *
 * @param dt Pointer to the dt_t object.
 * @param[out] valid Pointer to a boolean indicating if conversion was successful.
 * @return dt_t object representing the local time.
 */
dt_t dt_toLocalTime(const dt_t *dt, bool *valid);

/**
 * @brief Convert DateTime to UTC time.
 *
 * @param dt Pointer to the dt_t object.
 * @param[out] valid Pointer to a boolean indicating if conversion was successful.
 * @return dt_t object representing the UTC time.
 */
dt_t dt_toUTC(const dt_t *dt, bool *valid);

/**
 * @brief Get the year component of the DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @return Year component of the dt_t object.
 */
uint16_t dt_getYear(const dt_t *dt);

/**
 * @brief Get the month component of the DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @return Month component (1-12) of the dt_t object.
 */
uint8_t dt_getMonth(const dt_t *dt);

/**
 * @brief Get the day component of the DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @return Day component (1-31) of the dt_t object.
 */
uint8_t dt_getDay(const dt_t *dt);

/**
 * @brief Get the hour component of the DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @return Hour component (0-23) of the dt_t object.
 */
uint8_t dt_getHour(const dt_t *dt);

/**
 * @brief Get the minute component of the DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @return Minute component (0-59) of the dt_t object.
 */
uint8_t dt_getMinute(const dt_t *dt);

/**
 * @brief Get the second component of the DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @return Second component (0-59) of the dt_t object.
 */
uint8_t dt_getSecond(const dt_t *dt);

/**
 * @brief Get the millisecond component of the DateTime object.
 *
 * @param dt Pointer to the dt_t object.
 * @return Millisecond component (0-999) of the dt_t object.
 */
uint16_t dt_getMillisecond(const dt_t *dt);

/**
 * @brief Calculate the difference between two DateTime objects.
 *
 * @param dt1 Pointer to the first dt_t object.
 * @param dt2 Pointer to the second dt_t object.
 * @param[out] valid Pointer to a boolean indicating if calculation was successful.
 * @return Difference in milliseconds between the two dt_t objects.
 */
int64_t dt_diff(const dt_t *dt1, const dt_t *dt2, bool *valid);

/**
 * @brief Format the DateTime object using a custom format string.
 *
 * @param dt Pointer to the dt_t object.
 * @param format Format string (similar to strftime).
 * @param buffer Buffer to store the formatted string.
 * @param bufferSize Size of the buffer.
 * @param[out] valid Pointer to a boolean indicating if formatting was successful.
 */
void dt_format(const dt_t *dt, const char *format, char *buffer, size_t bufferSize, bool *valid);

#endif // DATETIME_H
