export nodes=$1
export rows=$2
export iterations=$3
export simCycles=$4
export dest_folder=$5
export benchmark=$6

echo "Running traffic corelation($benchmark benchmark) data for and rows : " $nodes " " $rows

export is_apply_cf=false
export cw=""
export cw_flag=""
if [[ "$apply_cf" == "-cw" ]]; then
    is_apply_cf=true 
    cw="c"
    cw_flag="--enable-add-chaff"
fi

export delay=""
export delay_flag=""
if [[ "$apply_delay" == "-delay" ]]; then
    delay="d"
    delay_flag="--enable-add-delay"
fi


for i in $( eval echo {0..$(($nodes-1))})
do
    for j in $( eval echo {0..$(($nodes-1))})
    do
        if [[ ("$is_apply_cf" = true ) && (($j -eq $(($i-1))) || ($j -eq $(($i+1))) || ($j -eq $(($i+$rows))) || ($j -eq $(($i-$rows))))]]; then
            continue
        fi
        if [ $i -eq $j ]; then
            continue
        fi
        for k in $( eval echo {0..$(($iterations-1))})
        do
            export def_mc=$(( $RANDOM % $nodes + 0 ))
            while [[ ($def_mc -eq $(($i))) || ($def_mc -eq $(($j))) ]]
            do
                export def_mc=$(( $RANDOM % $nodes + 0 ))
            done
            export out_filename="${nodes}_${i}_${j}_${k}.txt" 
            ../build/X86/gem5.opt -d $dest_folder/"${nodes}_nodes_${cw}${delay}_${benchmark}"/${i}_${j} --debug-file=$out_filename --debug-flag=Hello ../configs/example/se_1core.py --dir-mp-src1=$i --dir-mp-mem1=$j --dir-mp-default=$def_mc --num-cpus=$nodes --num-dir=$nodes --cpu-type=timing --cpu-clock=2GHz --caches --l1d_size=1kB --l1i_size=1kB --l2cache --num-l2caches=16 --l2_size=8kB --mem-type=RubyMemoryControl --mem-size=4GB --ruby --topology=Mesh_XY --mesh-rows=$rows --network=garnet2.0 --rel-max-tick=$simCycles -c "/gem5/gem5/dummy_pr;/gem5/gem5/benchmarks/${benchmark}"
        done
        rm -r $dest_folder/"${nodes}_nodes_${cw}${delay}_${benchmark}"/${i}_${j}/*.ini
        rm -r $dest_folder/"${nodes}_nodes_${cw}${delay}_${benchmark}"/${i}_${j}/*.json
    done
done



# for c-style 

# for (( i = 0; i < $nodes; i++))
# do
#     for (( j = 0; j < $nodes; j++))
#     do
#         if (($i == $j)); then
#             continue
#         fi
#         for (( k = 0; k < $iterations; k++))
#         do
#             # out_filename = "${nodes}_${i}_${j}_${k}.txt" 
#             # ../build/X86_DeepCorr/gem5.debug --debug-file=raw_dd/"${nodes}_nodes"/out_filename --debug-flags=GarnetSyntheticTraffic2,Hello ../configs/example/garnet_synth_traffic.py --cor-p1=$i --cor-p2=$j --num-cpus=4 --num-dirs=4 --network=garnet2.0 --topology=Mesh_XY --mesh-rows=2 --sim-cycles=1000  --synthetic=uniform_random --injectionrate=0.01
#             echo $i $j
#         done
#     done
# done