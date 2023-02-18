
"""
FILE: skeleton_parser.py
------------------
Author: Firas Abuzaid (fabuzaid@stanford.edu)
Author: Perth Charernwattanagul (puch@stanford.edu)
Modified: 04/21/2014

Skeleton parser for CS564 programming project 1. Has useful imports and
functions for parsing, including:

1) Directory handling -- the parser takes a list of eBay json files
and opens each file inside of a loop. You just need to fill in the rest.
2) Dollar value conversions -- the json files store dollar value amounts in
a string like $3,453.23 -- we provide a function to convert it to a string
like XXXXX.xx.
3) Date/time conversions -- the json files store dates/ times in the form
Mon-DD-YY HH:MM:SS -- we wrote a function (transformDttm) that converts to the
for YYYY-MM-DD HH:MM:SS, which will sort chronologically in SQL.

Your job is to implement the parseJson function, which is invoked on each file by
the main function. We create the initial Python dictionary object of items for
you; the rest is up to you!
Happy parsing!
"""

import sys
from json import loads
from re import sub

columnSeparator = "|"

# Dictionary of months used for date transformation
MONTHS = {'Jan':'01','Feb':'02','Mar':'03','Apr':'04','May':'05','Jun':'06',\
        'Jul':'07','Aug':'08','Sep':'09','Oct':'10','Nov':'11','Dec':'12'}

"""
Returns true if a file ends in .json
"""
def isJson(f):
    return len(f) > 5 and f[-5:] == '.json'

"""
Converts month to a number, e.g. 'Dec' to '12'
"""
def transformMonth(mon):
    if mon in MONTHS:
        return MONTHS[mon]
    else:
        return mon

"""
Transforms a timestamp from Mon-DD-YY HH:MM:SS to YYYY-MM-DD HH:MM:SS
"""
def transformDttm(dttm):
    dttm = dttm.strip().split(' ')
    dt = dttm[0].split('-')
    date = '20' + dt[2] + '-'
    date += transformMonth(dt[0]) + '-' + dt[1]
    return date + ' ' + dttm[1]

"""
Transform a dollar value amount from a string like $3,453.23 to XXXXX.xx
"""

def transformDollar(money):
    if money == None or len(money) == 0:
        return money
    return sub(r'[^\d.]', '', money)

"""
Parses a single json file. Currently, there's a loop that iterates over each
item in the data set. Your job is to extend this functionality to create all
of the necessary SQL tables for your database.
"""
def parseJson(json_file):
    with open(json_file, 'r') as f:
        items = loads(f.read())['Items'] # creates a Python dictionary of Items for the supplied json file
        table_dict = dict()
        table_dict["Item"] = list()
        table_dict["User"] = list()
        table_dict["Category"] = list()
        table_dict["Bid"] = list()
        for item in items:
            """
            TODO: traverse the items dictionary to extract information from the
            given `json_file' and generate the necessary .dat files to generate
            the SQL tables based on your relation design
            """
            curr_item_list = list()
            curr_user_list = list()
            curr_item_list.append(item["ItemID"])
            if("Name" in item.keys()):    
                curr_item_list.append(item["Name"])
            else:
                curr_item_list.append("NULL")
            for category in item["Category"]:
                curr_category_list = list()
                curr_category_list.append(item["ItemID"])
                curr_category_list.append(category)
                table_dict["Category"].append(curr_category_list)    
            if("Currently" in item.keys()):    
                curr_item_list.append(transformDollar(item["Currently"]))
            else:
                curr_item_list.append("NULL")
            if("Buy_Price" in item.keys()):                
                curr_item_list.append(transformDollar(item["Buy_Price"]))
            else:
                curr_item_list.append("NULL")    
            if("First_Bid" in item.keys()):                
                curr_item_list.append(transformDollar(item["First_Bid"]))
            else:
                curr_item_list.append("NULL")
            if("Number_of_Bids" in item.keys()):        
                curr_item_list.append(item["Number_of_Bids"])
            else:
                curr_item_list.append("NULL")    
            if(item["Bids"] != None):
                for bid in item["Bids"]:
                    curr_bid_list = list()
                    curr_bid_list.append(item["ItemID"])
                    curr_bid_list.append(bid["Bid"]["Bidder"]["UserID"])
                    if("Time" in bid["Bid"].keys()):
                        curr_bid_list.append(transformDttm(bid["Bid"]["Time"]))
                    else:
                        curr_bid_list.append("NULL")
                    curr_bid_list.append(transformDollar(bid["Bid"]["Amount"]))
                    table_dict["Bid"].append(curr_bid_list)
                    curr_user_list = list()
                    curr_user_list.append(bid["Bid"]["Bidder"]["UserID"])
                    if("Rating" in bid["Bid"]["Bidder"].keys()):   
                        curr_user_list.append(bid["Bid"]["Bidder"]["Rating"])
                    else:
                        curr_user_list.append("NULL")
                    if("Location" in bid["Bid"]["Bidder"]):        
                        curr_user_list.append(bid["Bid"]["Bidder"]["Location"])
                    else:
                        curr_user_list.append("NULL")
                    if("Country" in bid["Bid"]["Bidder"]):    
                        curr_user_list.append(bid["Bid"]["Bidder"]["Country"])
                    else:
                        curr_user_list.append("NULL")    
                    table_dict["User"].append(curr_user_list)
            curr_user_list.append(item["Seller"]["UserID"])        
            if("Rating" in item["Seller"].keys()):    
                curr_user_list.append(item["Seller"]["Rating"])
            else:
                curr_user_list.append("NULL")        
            if("Location" in item.keys()):        
                curr_user_list.append(item["Location"])
            else:
                curr_user_list.append("NULL")
            if("Country" in item.keys()):    
                curr_user_list.append(item["Country"])
            else:
                curr_user_list.append("NULL")
            if("Started" in item.keys()):    
                curr_item_list.append(transformDttm(item["Started"]))
            else:
                curr_item_list.append("NULL")
            if("Ends" in item.keys()):       
                curr_item_list.append(transformDttm(item["Ends"]))
            else:
                curr_item_list.append("NULL")
            if("Description" in  item.keys()):    
                curr_item_list.append(item["Description"])
            else:
                curr_item_list.append("NULL")        
            if("UserID" in item["Seller"].keys()):    
                curr_item_list.append(item["Seller"]["UserID"])
            else:
                curr_item_list.append("NULL")
            table_dict["Item"].append(curr_item_list)
            table_dict["User"].append(curr_user_list)   
        for key in table_dict.keys():
            with open(key + ".dat", "a") as fp:
                for row in table_dict[key]:
                    i = 0
                    for column in row:
                        if i>0:
                            fp.write("|")
                        if(column is None):
                            fp.write("NULL")
                        elif (hasQuotes(column)):
                            fp.write(escapeQuotes(column))
                        else:
                            fp.write(column)
                        i+=1    
                    fp.write("\n")        
    return        

def hasQuotes(column):
    for char in column:
        if char == '\"':
            return True
    return False      

def escapeQuotes(column):
    escapedColumn = "\""
    for char in column:
        escapedColumn += char
        if char == '\"':
            escapedColumn += char
    escapedColumn += '\"'
    return escapedColumn                 

                
            



"""
Loops through each json files provided on the command line and passes each file
to the parser
"""
def main(argv):
    if len(argv) < 2:
        print >> sys.stderr, 'Usage: python skeleton_json_parser.py <path to json files>'
        sys.exit(1)
    filenames = ["User.dat", "Item.dat", "Category.dat", "Bid.dat"]  
    for filename in filenames:
        with open(filename, "r+") as fp:
            fp.truncate(0)  
    # loops over all .json files in the argument
    for f in argv[1:]:
        if isJson(f):
            parseJson(f)
            print ("Success parsing " + f)
    # print(escapeQuotes("Ali\""))

if __name__ == '__main__':
    main(sys.argv)
