Kinetica FME Connector Changelog
================================

Version 6.2.0 -  2018-04-26
---------------------------

-   Changed default behavior of the writer to declare all columns (attributes)
    as nullable by default; changed the 'nullable' index property to 'not_nullable'
	(selecting which would make a column not nullable).  Primary key columns will
	not be nullable.
-   Added functionality to automatically convert date, datetime, and timestamp
    values to Kinetica's minimum and maximum values when the given value is
	out of range.  The valid ranges are:
	-   Timestamp: [-30610224000000, 29379542399999] (corresponding to
	               [1000-01-01 00:00:00.000, 2900-12-31 23:59:59.999] )
	-   Date:      [1000-01-01, 2900-12-31]
	-   DateTime:  [1000-01-01 00:00:00.000, 2900-12-31 23:59:59.999]
-   Added support for charN


Version 6.1.0 -  2018-01-05
---------------------------
-   Added support for the following types:
    -   date (string)
    -   datetime (string)
	-   time (string)
	-   timestamp (long; number of seconds from the epoch)
	-   decimal
	-   WKT
	-   IPv4
	-   int8
	-   int16
-   Added support for nullable columns
