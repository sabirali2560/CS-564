-- SQLite
drop table if exists User;
drop table if exists Item; 
drop table if exists Bid; 
drop table if exists Category;

CREATE TABLE User (
User_id CHAR(256) PRIMARY KEY, 
Rating INT, 
Location CHAR(256), 
Country CHAR(256)
);

CREATE TABLE Item ( 
Item_id INT PRIMARY KEY, 
Name CHAR(256), 
Currently FLOAT, 
Buy_price FLOAT,
First_bid FLOAT, 
Number_of_Bid INT, 
Started DATETIME, 
Ended DATETIME,
Description CHAR(1000), 
Seller_id CHAR(256), 
FOREIGN KEY (Seller_id) REFERENCES User 
);


CREATE TABLE Bid (
Item_id INT, 
Bidder_id CHAR(256), 
Time DATETIME, 
Amount FLOAT, 
PRIMARY KEY(Item_id, Bidder_id, Amount),
FOREIGN KEY (Item_id) REFERENCES Item, 
FOREIGN KEY (Bidder_id) REFERENCES User 
);

CREATE TABLE Category( 
Item_id INT, 
Category CHAR(256),
PRIMARY KEY (Item_id, Category) 
FOREIGN KEY (Item_id) REFERENCES Item
);