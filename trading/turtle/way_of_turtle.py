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
        historical = self.client.get_historical_klines(investment_target, Client.KLINE_INTERVAL_1DAY, "8 Jun 2022")
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
          
######## strategy #########

def strategy＿donchian＿channel(data):
    x1=data.Close>data.upper_band
    print(f"data.Close= {data.Close}")
    print(f"data.upper_band= {data.upper_band}")
    x2=data.Close.shift(1)<data.upper_band.shift(1)
    x=x1&x2
    y1=data.Close<data.lower_band
    y2=data.Close.shift(1)>data.lower_band.shift(1)
    y=y1&y2
    data.loc[x,'signal']='buy'
    data.loc[y,'signal']='sell'
    buy_date=pd.to_datetime(data.loc[data.signal == 'buy', 'Close time'], unit='s').dt.strftime('%Y-%m-%d')
    sell_date=pd.to_datetime(data.loc[data.signal == 'sell', 'Close time'], unit='s').dt.strftime('%Y-%m-%d')
    data.loc[x,'bc']=data[data.signal=='buy'].Close.round(2)
    data.loc[y,'sc']=data[data.signal=='sell'].Close.round(2)
    buy_close=data[data.signal=='buy'].Close.round(2).tolist()
    sell_close=data[data.signal=='sell'].Close.round(2).tolist()

    return (buy_date,buy_close,sell_date,sell_close)
    
    
########### main ############

if __name__ == '__main__':
    api_key = input('Insert your api_key')
    api_secret = input('Insert your api_secret')
    
    investment_target = 'BTCUSDT'    
    trading = trading_binance(api_key, api_secret)
    short_term = True
    
    prices = trading.get_daily_data(investment_target)

    history_prices = trading.get_history(investment_target)
    history_prices_describe = history_prices.describe()
    
    if (short_term):
        channel_len = 20
    else :
        channel_len = 55
    
    history_prices['upper_band'] = history_prices['High'].shift(1).rolling(channel_len).max()
    history_prices['lower_band'] = history_prices['Low'].shift(1).rolling(channel_len).min()
   
  
    bd,bc,sd,sc = strategy＿donchian＿channel(history_prices)
    


    add_plots = [
                mpf.make_addplot(history_prices['upper_band'], color='green'),
                mpf.make_addplot(history_prices['lower_band'], color='red'),
                mpf.make_addplot(history_prices['bc'].values, type='scatter', markersize=60, marker='^'),
                mpf.make_addplot(history_prices['sc'].values, type='scatter', markersize=60, marker='v')
                ]
    
    mpf.plot(
             history_prices.set_index('Close time'), 
             type='candle', 
             style='charles', 
             volume=True,
             title='BTCUSDT',
             mav=(20,55),
             addplot=add_plots
             )

    '''
    order = trading.create_test_order(
                                    symbol=investment_target,
                                    side=Client.SIDE_BUY,
                                    type=Client.ORDER_TYPE_MARKET,
                                    quantity=100
                                    )
    '''