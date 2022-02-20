from xml.etree import ElementTree as ET
import sys

tree=ET.parse(sys.argv[1])

root=tree.getroot()

# print(root.tag, root.attrib)

total_received_bytes=0
total_received_packets=0
start_time=0.0
end_time=0.0
packets_lost=0
total_sent_packets=0

for elem in root.findall('FlowStats/Flow'):
    total_received_packets+=int(elem.get('rxPackets'))
    total_sent_packets+=int(elem.get('txPackets'))
    total_received_bytes+=int(elem.get('rxBytes'))
    packets_lost+=int(elem.get('lostPackets'))

    start = float(elem.get('timeFirstTxPacket')[:-2])
    end = float(elem.get('timeLastRxPacket')[:-2])

    if(end > end_time):
        end_time=end
    
    if(start < start_time or start_time==0.0):
        start_time=start


print("total packets received:",total_received_packets)
print("total packets sent:",total_sent_packets)
print("total bytes received:",total_received_bytes)
print("total packets lost:",packets_lost)
packet_loss_ratio = (packets_lost*1.0/(total_sent_packets))*100
print("packet_loss_ratio(%):",packet_loss_ratio)

simulation_time  = (end_time-start_time)*1e-9 #in seconds
average_network_throughput = total_received_bytes*1.0/30 #bytes/second
print("simulation_time(sec):", simulation_time)
print("average network throughput(bytes/sec):", average_network_throughput)