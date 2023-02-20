SELECT COUNT(DISTINCT(u.User_id))
FROM User u, Item i, Bid b
WHERE u.User_id = i.Seller_id
AND u.User_id = b.Bidder_id;