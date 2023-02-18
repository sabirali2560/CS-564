# Commented part calculates and outputs the time required to run the script
# start=`date +%s`
python3 skeleton_parser.py ebay_data/items-*.json
uniq User.dat > /dev/null
sort User.dat > /dev/null
uniq Item.dat > /dev/null
sort Item.dat > /dev/null
uniq Bid.dat > /dev/null
sort Bid.dat > /dev/null
uniq Category.dat > /dev/null
sort Category.dat > /dev/null
# end=`date +%s`
# runtime=$( echo "$end - $start" | bc -l )
# echo $runtime