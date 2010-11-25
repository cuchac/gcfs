#define GCFS_CONFIG_DEFAULTCONFIG		\
"# This is configuration file for Gcfs - Grid Control File System\n\
# Comments start with symbol '#'\n\
# Uncoment and modify values\n\
\n\
#### Global settings #####\n\
[Global]\n\
## Path where temporary files will be stored\n\
#data_path=/home/joe/.local/gcfs/\n\
#data_path=/tmp/gcfs/\n\
\n\
## List services\n\
service=Condor\n\
#service=Globus\n\
\n\
## Default service that will be used for creaed tasks\n\
default_service=Condor\n\
\n\n\
#### Services configuration ####\n\
## Every category configures one service defined above\n\
[Condor]\n\
## Driver defines the grid framework to use\n\
## Currently supported only SAGA\n\
driver=saga\n\
\n\
## Saga service url - URL of resource manager\n\
service_url=condor://localhost/\n\
\n\
## Here you can specify default configuration values of tasks of given service\n\
[Condor.default]\n\
## Each 'key=value' pair sets config variable 'key' to 'value'\n\
architecture=x86\n\
memory=123\n\
\n\
## Here you can specify default environment variables\n\
[Condor.environment]\n\
## Each 'key=value' pair sets environment variable 'key' to 'value'\n\
SUBMITTER=GCFS\n\
"
