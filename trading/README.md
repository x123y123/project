# Trading with Binance

Device: Macbook M1 air

## ENV
* conda
Install conda for arm64 (Apple Silicon) [Miniforge3-MacOSX-arm64](https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-arm64.sh)
```shell
$ cd ~/Download
$ sh Miniforge3-MacOSX-arm64.sh
$ source ~/.zshrc
$ conda activate base
$ conda create --name trading
$ conda activate trading
$ conda install python
# conda install python=3.8

# leave trading env
$ conda deactive
```
* pip install
```shell
$ pip install python-binance pandas mplfinance backtrader
```

## Reference 
* [miniforge Github](https://github.com/conda-forge/miniforge#download)
* [python-binance](https://python-binance.readthedocs.io/en/latest/)
