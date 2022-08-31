export noise_ratio=$1

#python3 transform_data_to_numpy_2_pair.py --no-of-nodes=64_nodes__FFT_FMM_${noise_ratio} --base-path=/export/research27/tornado/hansika/noc_data/ --cal-dir-path=calculated/ --rawd-dir-path=raw_data/ --numpy-dir-path=numpy_data/ --min-max-length=250

python3 transform_data_to_numpy_2_pair.py --no-of-nodes=64_nodes__FMM_LU_${noise_ratio} --base-path=/export/research27/tornado/hansika/noc_data/ --cal-dir-path=calculated/ --rawd-dir-path=raw_data/ --numpy-dir-path=numpy_data/ --min-max-length=250

python3 transform_data_to_numpy_2_pair.py --no-of-nodes=64_nodes__LU_BARNES_${noise_ratio} --base-path=/export/research27/tornado/hansika/noc_data/ --cal-dir-path=calculated/ --rawd-dir-path=raw_data/ --numpy-dir-path=numpy_data/ --min-max-length=250

python3 transform_data_to_numpy_2_pair.py --no-of-nodes=64_nodes__BARNES_RADIX_${noise_ratio} --base-path=/export/research27/tornado/hansika/noc_data/ --cal-dir-path=calculated/ --rawd-dir-path=raw_data/ --numpy-dir-path=numpy_data/ --min-max-length=250

python3 transform_data_to_numpy_2_pair.py --no-of-nodes=64_nodes__RADIX_FFT_${noise_ratio} --base-path=/export/research27/tornado/hansika/noc_data/ --cal-dir-path=calculated/ --rawd-dir-path=raw_data/ --numpy-dir-path=numpy_data/ --min-max-length=250