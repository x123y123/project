import pandas as pd
import numpy as np
import mplfinance as mpf
import IPython.display as IPydisplay
import ta
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
        rsi = ta.momentum.RSIIndicator(hist_df['Close'], window=14)
        hist_df['rsi'] = rsi.rsi()
        #print(hist_df.info())
        return hist_df
    
    def buy_sell(self, data,signal):
        # Get futures account balance
        balances = self.client.futures_account_balance()

        # Find USDT balance in the balance list
        usdt_balance = next(item['balance'] for item in balances if item['asset'] == 'USDT')

        # Print the USDT balance
        print(f"USDT balance: {usdt_balance}")
        
        # Unit=（0.01 * account_total）/ ATR
        #data['unit'] = 0.01 * float(usdt_balance) / data['ATR']
        data['unit'] = 0.01 * 100.0 / data['ATR']

######## indicator #########
    
def check_rsi(rsi):
    return (rsi > 70) or (rsi < 30)
        
######## strategy #########

def strategy＿donchian＿channel(data):
    x1=data.Close>data.upper_band
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
    
    
################################################################
#           .___  ___.      ___       __  .__   __.            #
#           |   \/   |     /   \     |  | |  \ |  |            #
#           |  \  /  |    /  ^  \    |  | |   \|  |            #
#           |  |\/|  |   /  /_\  \   |  | |  . `  |            #
#           |  |  |  |  /  _____  \  |  | |  |\   |            #
#           |__|  |__| /__/     \__\ |__| |__| \__|            #
################################################################                                       

if __name__ == '__main__':
    api_key = input('Insert your api_key')
    api_secret = input('Insert your api_secret')
    
    investment_target = 'BTCUSDT'    
    trading = trading_binance(api_key, api_secret)
    short_term = True
    if (short_term):
        channel_len = 20
    else :
        channel_len = 55
        
    prices = trading.get_daily_data(investment_target)

    history_prices = trading.get_history(investment_target)
    #history_prices_describe = history_prices.describe()


########### Is in Correction? ##########

    rsi_check = history_prices['rsi'].apply(check_rsi)
    if rsi_check is True:
        print('turtle')
    else:
        print('Correction')
        
################# WMS ##################
    
    # Calculate TR
    history_prices['TR'] = history_prices[['High', 'Close']].max(axis=1) - history_prices[['Low', 'Close']].min(axis=1)
    history_prices['TR1'] = abs(history_prices['High'] - history_prices['Close'].shift(-1))
    history_prices['TR2'] = abs(history_prices['Low'] - history_prices['Close'].shift(-1))
    history_prices['TR'] = history_prices[['TR', 'TR1', 'TR2']].max(axis=1)
    history_prices.drop(['TR1', 'TR2'], axis=1, inplace=True)
    
    # Calculate ATR
    history_prices['ATR'] = history_prices['TR'].rolling(channel_len).mean()
    
    trading.buy_sell(history_prices, 1)
   
    

################# buying signal & sell signal ####################
    
   
    
    history_prices['upper_band'] = history_prices['High'].shift(1).rolling(channel_len).max()
    history_prices['lower_band'] = history_prices['Low'].shift(1).rolling(channel_len).min()
   
  
    bd,bc,sd,sc = strategy＿donchian＿channel(history_prices)
    

############# plot ##############

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
             #mav=(20,55),
             addplot=add_plots,
             savefig='./img/donchian＿channel.png'
             )
    IPydisplay.Image(filename='./img/donchian＿channel.png')

    '''
    order = trading.create_test_order(
                                    symbol=investment_target,
                                    side=Client.SIDE_BUY,
                                    type=Client.ORDER_TYPE_MARKET,
                                    quantity=100
                                    )
    '''