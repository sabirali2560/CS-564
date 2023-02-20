SELECT COUNT(DISTINCT(u.User_id))
FROM User u, Item i
WHERE u.User_id = i.Seller_id
AND u.Rating > 1000;