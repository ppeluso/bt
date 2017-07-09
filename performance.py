import pandas as pd 
import sqlite3
import matplotlib.pyplot as plt
class Performance:

    def __init__(self, path_to_db, table_name, signal_table):

        self.path_to_db = path_to_db
        self.table_name = table_name
        self.conn = sqlite3.connect(self.path_to_db)
        self.df = pd.read_sql_query("SELECT * FROM {0} ;".format(table_name), self.conn)
        self.signals = pd.read_sql_query("SELECT * FROM {0}".format(signal_table), self.conn)
        self.signals["Date"] = pd.to_datetime(self.signals["Date"])
    def plot(self):
    	fig, ax = plt.subplots()
    	ax.plot(self.signals["Date"], self.df["val"])
    	ax.plot(self.signals["Date"][self.signals["RDS_A"] == 100], self.df["val"][self.signals["RDS_A"] == 100], '^', color='green', markersize=10)
    	ax.plot(self.signals["Date"][self.signals["RDS_A"] == -100], self.df["val"][self.signals["RDS_A"] == -100], '^', color='red', markersize=10)
    	plt.show()
    def close_db(self):
    	self.conn.close()
if __name__ == '__main__':

    p = Performance('test.db', 'pl', 'strat')
    p.plot()

