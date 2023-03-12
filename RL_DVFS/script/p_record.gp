reset
set title 'Power in 200 episode'
set ylabel 'Power (mW)'

set xlabel 'episode (times)'

set term png enhanced font 'Times_New_Roman,12'

set output "power_record.png"

set format x '%10.0f'
set xtics 0, 10 
set xtics rotate by 45 right


plot [0:200][0:5]"p_record.txt"using 0:1 with lines title 'cpu Power',\
