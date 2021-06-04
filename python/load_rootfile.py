import os
import uproot
import numpy as np
import datetime 
import matplotlib.pyplot as plt

datapath = os.path.join("~", "projects", "data", "xbox2")
#filename = "Xbox3_T24N5_L1.root"
filename = "Xbox2_T24PSI_2_EventData_201804.root"
filename = "Xbox2_T24PSI_2.root"

#datapath = os.path.join("~", "programming", "uproot")
#filename = "test.root"
filepath = os.path.join(datapath, filename)


uproot.open(filepath)
file =  uproot.open(filepath)
print file.keys()

# take one specific tree from root file .....................................
tree = file['EventData']
tree.keys() # list all channels

channel = tree["PSI_amp"] # chose a channel
print channel.keys() # list all elements of the selected channel
       
# read data time stamps of all entries
sec = channel["fTimeStamp.fSec"].array() # 
nsec = channel["fTimeStamp.fNanoSec"].array()

# get time stamps of all events of current tree 
ts = [datetime.datetime.fromtimestamp(s) for s in sec]

# conversion of signal raw data to real data
rawdata = channel["fRawData"].array()

# choose signal of first event
rawdata0 = rawdata[0]

data = rawdata0.view(dtype=np.double) # xbox2
#data = rawdata0.view(dtype=np.short) # xbox3


# plot channel data of last event
fig1 = plt.figure()
ax = fig1.add_subplot(1, 1, 1)
ax.plot(data, linewidth=0.8, label="PSI AMP")

fig1.savefig("test2.pdf", format="pdf")
plt.close()
