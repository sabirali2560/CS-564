SELECT COUNT(*) FROM (
SELECT *
FROM Item i, Category c
WHERE i.Item_id = c.Item_id
GROUP BY i.Item_id
HAVING COUNT() = 4
);