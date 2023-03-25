import pandas as pd
import numpy as np
import mplfinance as mpf
from binance import Client, ThreadedWebsocketManager, ThreadedDepthCacheManager

######### function ########

class trading_binance:
    def __init__(self, api_key, api_secret):
        self.client = Client(api_key, api_secret)
        
    def get_daily_data(self, investment_target):
        prices = self.client.get_ticker(symbol=investment_target)
        return prices
    def get_history(self, investment_target):
        historical = self.client.get_historical_klines(investment_target, Client.KLINE_INTERVAL_1DAY, "8 Jun 2011")
        hist_df = pd.DataFrame(historical)
        hist_df.columns = [ # TB means taker buy
                            'Open time', 'Open', 'High', 'Low', 
                            'Close', 'Volume', 'Close time', 'Quote Asset Volume',
                            'Number of trades', 'TB base asset volume',         
                            'TB quote asset volume', 'Ignore'
                          ]
        hist_df['Open time'] = pd.to_datetime(hist_df['Open time']/1000, unit='s')
        hist_df['Close time'] = pd.to_datetime(hist_df['Close time']/1000, unit='s')
        
        numeric_col = [
                        'Open', 'High', 'Low', 'Close',
                        'Volume', 'Quote Asset Volume',
                        'TB base asset volume', 
                        'TB quote asset volume'
                      ]
        hist_df[numeric_col] = hist_df[numeric_col].apply(pd.to_numeric, axis=1)
        #print(hist_df.info())
        return hist_df
          

    
########### main ############

if __name__ == '__main__':
    api_key = input('Insert your api_key')
    api_secret = input('Insert your api_secret')
    
    investment_target = 'BTCUSDT'
    
    trading = trading_binance(api_key, api_secret)
    
    prices = trading.get_daily_data(investment_target)

    history_prices = trading.get_history(investment_target)
    history_prices_describe = history_prices.describe()
    
    mpf.plot(
             history_prices.set_index('Close time'), 
             type='candle', 
             style='charles', 
             volume=True,
             title='BTCUSDT',
             mav=(20,55)
             )
    
    '''
    order = trading.create_test_order(
                                    symbol=investment_target,
                                    side=Client.SIDE_BUY,
                                    type=Client.ORDER_TYPE_MARKET,
                                    quantity=100
                                    )
    '''