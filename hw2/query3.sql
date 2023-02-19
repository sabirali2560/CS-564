SELECT COUNT(*) FROM (
SELECT *
FROM Item, Category
WHERE Item.itemID = Category.itemID
GROUP BY Item.itemID
HAVING COUNT() = 4
);