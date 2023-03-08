reset
set title 'Action selection in 100 episode'
set ylabel 'Action (MHz)'

set xlabel 'episode (times)'

set term png enhanced font 'Times_New_Roman,12'

set output "action_record.png"

set format x '%10.0f'
set xtics 0, 10 
set xtics rotate by 45 right


plot [0:100][100:2000]"action_record.txt"using 0:1 with lines title 'Action (cpufreq)',\
