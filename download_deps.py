import os
import zipfile
from urllib.request import urlretrieve

dest_dir = 'dependencies'

if not os.path.exists(dest_dir):
    os.makedirs(dest_dir)

# Download tensorflow library

tf_lib_url = 'https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-windows-x86_64-2.3.1.zip'

tf_lib_dest_filename = tf_lib_url[tf_lib_url.rfind('/')+1:]
tf_lib_dest_fpath = os.path.join(dest_dir, tf_lib_dest_filename)

if not os.path.exists(tf_lib_dest_fpath):
    print('Downloading', tf_lib_dest_filename)
    print(tf_lib_url)
    urlretrieve(tf_lib_url, tf_lib_dest_fpath)

zip_ref = zipfile.ZipFile(tf_lib_dest_fpath, 'r')

required_files = []

for zip_file_ref in zip_ref.filelist:
    if zip_file_ref.filename.startswith('lib') or zip_file_ref.filename.startswith('include'):
        required_files.append(zip_file_ref)

for zip_file_ref in required_files:
    extract_dest = os.path.join(dest_dir, zip_file_ref.filename)
    if not os.path.exists(extract_dest):
        print('Inflating', extract_dest)

        extract_dest_dir, _ = os.path.split(extract_dest)

        if not os.path.exists(extract_dest_dir):
            os.makedirs(extract_dest_dir)

        with open(extract_dest, mode='wb') as outfile:
            outfile.write(zip_ref.read(zip_file_ref.filename))

    else:
        print('Skipping', extract_dest)

zip_ref.close()

# Download inception graph for demo
inception_graph_archive_url = 'https://storage.googleapis.com/download.tensorflow.org/models/inception5h.zip'
inception_graph_archive_filename = inception_graph_archive_url[inception_graph_archive_url.rfind('/')+1:]
inception_graph_archive_fpath = os.path.join(dest_dir, inception_graph_archive_filename)

if not os.path.exists(inception_graph_archive_fpath):
    print('Downloading', inception_graph_archive_filename)
    print(inception_graph_archive_url)
    urlretrieve(inception_graph_archive_url, inception_graph_archive_fpath)

inception_graph_entry_name = 'tensorflow_inception_graph.pb'
inception_graph_extracted_fpath = os.path.join(dest_dir, inception_graph_entry_name)

if not os.path.exists(inception_graph_extracted_fpath):
    zip_ref = zipfile.ZipFile(inception_graph_archive_fpath, 'r')

    with open(inception_graph_extracted_fpath, mode='wb') as outfile:
        outfile.write(zip_ref.read(inception_graph_entry_name))

    zip_ref.close()
