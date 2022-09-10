---
sidebar_label: Python
title: Connect with Python Connector
description: Connect to TDengine cloud service using Python connector
---

<!-- exclude -->
import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

<!-- exclude-end -->
## Install Connector

First, you need to install the `taospy` module version >= `2.6.2`. Run the command below in your terminal.

<Tabs defaultValue="pip">
<TabItem value="pip" label="pip">

```
pip3 install -U taospy
```
You'll need to have Python3 installed.

</TabItem>
<TabItem value="conda" label="conda">

```
conda install -c conda-forge taospy
```

</TabItem>
</Tabs>

## Config

Run this command in your terminal to save TDengine cloud token and URL as variables:

<Tabs defaultValue="bash">
<TabItem value="bash" label="Bash">

```bash
export TDENGINE_CLOUD_TOKEN="<token>"
export TDENGINE_CLOUD_URL="<url>"
```

</TabItem>
<TabItem value="cmd" label="CMD">

```bash
set TDENGINE_CLOUD_TOKEN="<token>"
set TDENGINE_CLOUD_URL="<url>"
```

</TabItem>
<TabItem value="powershell" label="Powershell">

```powershell
$env:TDENGINE_CLOUD_TOKEN="<token>"
$env:TDENGINE_CLOUD_URL="<url>"
```

</TabItem>
</Tabs>


Alternatively, you can also set environment variables in your IDE's run configurations.


<!-- exclude -->
:::note
Replace  <token\> and <url\> with cloud token and URL.
To obtain the value of cloud token and URL, please log in [TDengine Cloud](https://cloud.tdengine.com) and click "Connector" and then select "Python".

:::
<!-- exclude-end -->

## Connect

Copy code bellow to your editor and run it.

```python
{{#include docs/examples/python/develop_tutorial.py:connect}}
```

For how to write data and query data, please refer to <https://docs.tdengine.com/cloud/data-in/insert-data/> and <https://docs.tdengine.com/cloud/data-out/query-data/>.