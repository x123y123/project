reset
set title '[testing stage] State observation'
set ylabel 'state (CPI)'

set xlabel 'episode (times)'

set term png enhanced font 'Times_New_Roman,12'

set output "state_record_r.png"

set format x '%10.0f'
set xtics 0, 10 
set xtics rotate by 45 right


plot [0:200][0:20]"cpi_record.txt"using 0:1 with lines title 'cpi',\
