main:
	sh runParser.sh
	sqlite3 ebay.db < create.sql
	sqlite3 ebay.db < load.txt

test: 
	sqlite3 ebay.db < query1.sql
	sqlite3 ebay.db < query2.sql
	sqlite3 ebay.db < query3.sql
	sqlite3 ebay.db < query4.sql
	sqlite3 ebay.db < query5.sql
	sqlite3 ebay.db < query6.sql
	sqlite3 ebay.db < query7.sql