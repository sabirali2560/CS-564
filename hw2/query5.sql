SELECT COUNT(DISTINCT(u.UserID))
FROM User u, Item
WHERE u.UserID = Item.SellerID
AND u.Rating > 1000;