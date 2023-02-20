# Commented part calculates and outputs the time required to run the script
# start=`date +%s`
python3 skeleton_parser.py ebay_data/items-*.json
sort -u -o User.dat User.dat
sort -u -o Item.dat Item.dat
sort -u -o Bid.dat Bid.dat
sort -u -o Category.dat Category.dat
# end=`date +%s`
# runtime=$( echo "$end - $start" | bc -l )
# echo $runtime