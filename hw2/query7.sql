SELECT COUNT(DISTINCT(c.Category))
FROM Category c, Bid
WHERE c.ItemID = Bid.ItemID
AND Bid.Amount > 100;