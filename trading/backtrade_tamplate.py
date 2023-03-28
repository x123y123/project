import backtrader as bt
import datetime as dt

class BitcoinFeed(bt.feeds.GenericCSVData):
    params = (('high', 1),('low',1),('close',1),('volume', 2),('openinterest',-1))

import backtrader as bt


# Create a Stratey
class TestStrategy(bt.Strategy):

    def log(self, txt, dt=None):
        ''' Logging function for this strategy'''
        dt = dt or self.datas[0].datetime.date(0)
        print('%s, %s' % (dt.isoformat(), txt))

    def __init__(self):
        # Keep a reference to the "close" line in the data[0] dataseries
        self.dataclose = self.datas[0].close

    def next(self):
        # Simply log the closing price of the series from the reference
        self.log('Close, %.2f' % self.dataclose[0])

        '''
                _______.___________..______          ___   .___________. _______   ___________    ____ 
                /       |           ||   _  \        /   \  |           ||   ____| /  _____\   \  /   / 
               |   (----`---|  |----`|  |_)  |      /  ^  \ `---|  |----`|  |__   |  |  __  \   \/   /  
                \   \       |  |     |      /      /  /_\  \    |  |     |   __|  |  | |_ |  \_    _/   
            .----)   |      |  |     |  |\  \----./  _____  \   |  |     |  |____ |  |__| |    |  |     
            |_______/       |__|     | _| `._____/__/     \__\  |__|     |_______| \______|    |__|                                                                                                 
        '''

if __name__ == '__main__':

    cerebro = bt.Cerebro()
    print('Starting Portfolio Value: %.2f' % cerebro.broker.getvalue())
    
    data = bt.feeds.YahooFinanceCSVData(
                                        dataname='BTC-USD.csv',
                                        # Do not pass values before this date
                                        fromdate=dt.datetime(2000, 1, 1),
                                        # Do not pass values after this date
                                        todate=dt.datetime(2000, 12, 31),
                                        reverse=False
                                        )

    cerebro.adddata(data)
    cerebro.addstrategy(TestStrategy)
    cerebro.run()
    print('Final Portfolio Value: %.2f' % cerebro.broker.getvalue())
