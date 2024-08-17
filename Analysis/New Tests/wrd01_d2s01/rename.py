from glob import glob
import os

cmb = '11'

files = glob(f'board_{cmb}_ch_*.csv')
print(files)

for file in files:
    if 'NaN' in file:
        continue
    ch = file.split('.csv')[0][-2:]
    new_name = f'cmb{cmb}_ch{ch}.csv'
    print(f'moving {file} to {new_name}')
    os.system(f'mv {file} {new_name}')
