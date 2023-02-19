SELECT COUNT(DISTINCT(u.UserID))
FROM User u, Item, Bid
WHERE u.UserID = Item.SellerID
AND u.UserID = Bid.BidderID;