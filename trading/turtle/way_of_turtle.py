import pandas as pd
import numpy as np
#import talib as ta
#from datatime import datatime, timedelta
#import matplot.pyplot as plt
# show chinese and minus
#%matplotlib inline
from pylab import mpl
from binance import Client, ThreadedWebsocketManager, ThreadedDepthCacheManager

######### function ########

class trading_binance:
    def __init__(self, api_key, api_secret):
        self.client = Client(api_key, api_secret)
        
    def get_daily_data(self, investment_target):
        prices = self.client.get_ticker(symbol=investment_target)
        return prices
    def get_history(self, investment_target):
        history_prices = []
        for kline in self.client.get_historical_klines_generator(investment_target, Client.KLINE_INTERVAL_1MINUTE, "1 day ago UTC"):
            history_prices.append(kline)
        return history_prices
    
########### main ############

if __name__ == '__main__':

    api_key = input('Insert your api_key')
    api_secret = input('Insert your api_secret')
    
    investment_target = 'BTCUSDT'
    
    trading = trading_binance(api_key, api_secret)
    
    prices = trading.get_daily_data(investment_target)

    history_prices = trading.get_history(investment_target)
    
    
    '''
    order = trading.create_test_order(
                                    symbol=investment_target,
                                    side=Client.SIDE_BUY,
                                    type=Client.ORDER_TYPE_MARKET,
                                    quantity=100
                                    )
    '''