from requests import session, codes
from os import listdir, remove
from os.path import isfile, join
mypath = '/home/pi/Desktop/SweepData/'
path_files = [f for f in listdir(mypath) if isfile(join(mypath, f))]

def is_float(value):
    try:
        float(value)
    except ValueError:
        return False
    return True

session = session()
for file in path_files:
    f = open(mypath + file, 'rb')
    coords = file.strip('.txt').split('_')
    if len(coords) < 2:
        print('No Coordinates on File: ' + file)
        print('File will not be uploaded')
        continue
    lat = coords[-2]
    lon = coords[-1]
    upload_form = {
        'file': (file, f),
        'key': (None, 'Vy6nY#Q$p5x'),
    }
    result = session.post(url='https://rfmap.comtech.ncsu.edu/upload.php', files=upload_form)
    if result.status_code == codes.ok:
        f.close()
        remove(mypath + file)
    else:
        print('File Posting Error, Requests Status Code = ' + result.status_code)
