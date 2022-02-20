# /bin/bash

#main
IFS=$'\n'  
ns_output=( ` ./waf --run "scratch/red_simulator --queueDiscType=RED --nLeaf=$1 --tracing=true" ` )
ns_output=( ` ./waf --run "scratch/red_simulator --queueDiscType=ARED --nLeaf=$1 --tracing=true" ` )

#plot packet drops
gnuplot -persist <<-EOFMarker
    set title "time vs. Throughput"

    set xlabel "time"

    set ylabel "Throughput( bytes/sec )"

    plot "RED_ITP.dat" with lines, "ARED_ITP.dat" with lines

    pause -1 "Hit Enter to continue\n" 
EOFMarker

gnuplot -persist <<-EOFMarker
    set title "time vs. Packet Loss Ratio"

    set xlabel "time"

    set ylabel "Packet Loss Ratio( % )"

    plot "RED_IPL.dat" with lines, "ARED_IPL.dat" with lines

    pause -1 "Hit Enter to continue\n" 
EOFMarker






