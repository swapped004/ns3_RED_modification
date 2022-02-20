# /bin/bash

#main
current_dir=`pwd`
echo "$current_dir"

topology="wired"
usage="./red_simulator.sh -topology [-no_of_flows]"

# if (($# < 1));then
#     echo "Your command should look something like this:"
#     echo "      $usage"
#     exit
# fi

arr=([0]=5 [1]=10 [2]=15 [3]=20 [4]=25 [5]=30 [6]=35 [7]=40 [8]=45 [9]=50 [10]=55 [11]=60 [12]=65 [13]=70 [14]=75 [15]=80 [16]=85 [17]=90 [18]=95 [19]=100)

`touch "RED_pack_loss.dat"`
echo "# X   Y" > "RED_pack_loss.dat"

`touch "ARED_pack_loss.dat"`
echo "# X   Y" > "ARED_pack_loss.dat"

`touch "RED_TP.dat"`
echo "# X   Y" > "RED_TP.dat"

`touch "ARED_TP.dat"`
echo "# X   Y" > "ARED_TP.dat"

algos=([0]="RED" [1]="ARED")

for(( k=0; k<2; k++ ))
do
    for (( i=0; i<${#arr[@]}; i++ ))
    do
        echo "number of nodes: ${arr[$i]}"

        IFS=$'\n'  
        ns_output=( ` ./waf --run "scratch/red_simulator --queueDiscType=${algos[$k]} --nLeaf=${arr[$i]}" ` )
        # echo "ns3 simulation output:"
        # echo " ${ns_output[*]}"

        IFS=$'\n' 
        flow_monitor=( `python scratch/red_flow_parse.py red_flow.xml` )
        echo "${flow_monitor[*]}"

        packet_loss_ratio=( `echo "${flow_monitor[*]}" | grep -i "packet_loss_ratio" | rev | cut -d ' ' -f 1 | cut -c 2- | rev`)
        echo "packet loss ratio(%): "$packet_loss_ratio

        avg_network_throughput=( `echo "${flow_monitor[*]}" | grep -i "throughput" | rev | cut -d ' ' -f 1 | cut -c 2- | rev`)
        echo "TP: "$avg_network_throughput

        `echo "  ${arr[$i]} $packet_loss_ratio" >> ${algos[$k]}_pack_loss.dat `
        `echo "  ${arr[$i]} $avg_network_throughput" >> ${algos[$k]}_TP.dat `

    done
done


#plot packet drops
gnuplot -persist <<-EOFMarker
    set title "nodes vs. Packet Loss Ratio"

    set xlabel "nodes"

    set ylabel "Packet Loss Ratio( % )"

    plot "RED_pack_loss.dat" with lines, "ARED_pack_loss.dat" with lines

    pause -1 "Hit Enter to continue\n" 
EOFMarker

gnuplot -persist <<-EOFMarker
    set title "nodes vs. Throughput"

    set xlabel "nodes"

    set ylabel "Throughput( bytes/sec )"

    plot "RED_TP.dat" with lines, "ARED_TP.dat" with lines

    pause -1 "Hit Enter to continue\n" 
EOFMarker






