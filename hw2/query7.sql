SELECT COUNT(DISTINCT(c.Category))
FROM Category c, Bid b
WHERE c.Item_id = b.Item_id
AND b.Amount > 100;