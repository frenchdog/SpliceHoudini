# Quick and (very) dirty script to get the latest Fabric build
import os
import urllib2
from urllib2 import HTTPError, URLError
from HTMLParser import HTMLParser
import subprocess
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-i", "--installPath", dest="installPath",
                  help="Path to extract the zip file. Default is: ~/Dev/Fabric/", default="~/Dev/Fabric/")
(options, args) = parser.parse_args()
installPath = os.path.expanduser(options.installPath)

zipFileName = False


def unzip(zipPath, installRoot):
    p = subprocess.Popen(
        ['unzip', zipPath, '-d', installRoot], stdout=subprocess.PIPE)
    print p.communicate()[0]
    return p.returncode


class MyHTMLParser(HTMLParser):

    def handle_starttag(self, tag, attrs):
        global zipFileName
        if not zipFileName and len(attrs) == 1 and len(attrs[0]) == 2:
            name = attrs[0][1]
            if(len(name.split(".zip")) > 1):
                zipFileName = attrs[0][1]

try:
    dailiesUrl = "http://dist.fabric-engine.com/FabricEngine/pablo/Darwin/x86_64/Release/"
    sock = urllib2.urlopen(dailiesUrl)
    htmlSource = sock.read()
    sock.close()

    MyHTMLParser().feed(htmlSource)

    if zipFileName:
        zipUrl = dailiesUrl + zipFileName
        sock = urllib2.urlopen(zipUrl)
        if sock.info()["Content-Type"] == "application/zip":
            print sock.info()

        dailyBuildZipPath = os.path.join(installPath, zipFileName)
        fileName = os.path.splitext(zipFileName)[0]
        dailyBuildPath = os.path.join(installPath, fileName)

        if not os.path.exists(dailyBuildPath):

            # download zip file
            with open(dailyBuildZipPath, 'w') as outfile:
                print "Downloading %s to %s" % (zipFileName, installPath)
                outfile.write(sock.read())

            # If zip file not extracted
            if not os.path.exists(dailyBuildPath):
                print "Extracting..."
                rt = unzip(dailyBuildZipPath, installPath)
                if rt != 0:
                    print "Error happened while extracting %s. Installation aborted. Error: %s" % options.zipPath

        sock.close()

        with open(os.path.join(dailyBuildPath, "git-versions"), 'r') as gitVersions:
            print "List of projects using %s:" % fileName
            print gitVersions.read()

except HTTPError, e:
    print "HTTP Error:", e.code
except URLError, e:
    print "URL Error:", e.reason
