### batch
* route:`<xx>/objects/batch`
* method:`POST`

```
//request
{
    "transfers": [
        "a"
    ],
    "operation": "",
    "objects": [
        {
            "oid": "",
            "size": 0,
        }
    ]
}

//reponse
{
""
[
{
    "oid": "",
    "size": 0,
    "actions": {
        "download": {
            "href": "<xx>/objects/<oid>",
            "header": {
                "Accept": "application/vnd.git-lfs",
                "Authorization":"xx:xx"
            },
            "expires_at": "2016-10-02T15:32:32.087972164+08:00"
        },
        "upload": {
            "href": "<xx>/objects/<oid>",
            "header": {
                "Accept": "application/vnd.git-lfs",
                "Authorization":"xx:xx"
            },
            "expires_at": "2016-10-02T15:32:32.087972164+08:00"
        }
    },
    "error": {
        "code": 0,
        "message": ""
    }
}
]
}

```

### objects/oid
* route:`<xx>/objects/<oid>`

* `GET/HEAD` `Accept:application/vnd.git-lfs;`

```
//request
{
	"oid": "",
	"size": 0,
}

//reponse
<binary data>
```

* `GET/HEAD` `Accept`:`application/vnd.git-lfs+json;`

```
//request
{
	"oid": "",
	"size": 0,
}

//reponse, return head(HEAD) or json data(GET)
{
    "oid": "",
    "size": 0,
    "actions": {
        "download": {
            "href": "<xx>/objects/<oid>",
            "header": {
                "Accept": "application/vnd.git-lfs",
                "Authorization":"xx:xx"
            },
            "expires_at": "2016-10-02T15:32:32.087972164+08:00"
        }
    },
    "error": {
        "code": 0,
        "message": ""
    }
}
```

* `PUT` `Accept`:`application/vnd.git-lfs;`

```
//request
<binary data>

//reponse
<empty>
```

### objects
* route:`<xx>/objects`
* method:`POST`
```
//request
{
	"oid": "",
	"size": 0,
}

//reponse, return 200:exist,202:not exist
{
    "oid": "",
    "size": 0,
    "actions": {
        "download": {
            "href": "<xx>/objects/<oid>",
            "header": {
                "Accept": "application/vnd.git-lfs",
                "Authorization":"xx:xx"
            },
            "expires_at": "2016-10-02T15:32:32.087972164+08:00"
        },
        "upload": {
            "href": "<xx>/objects/<oid>",
            "header": {
                "Accept": "application/vnd.git-lfs",
                "Authorization":"xx:xx"
            },
            "expires_at": "2016-10-02T15:32:32.087972164+08:00"
       }
    },
    "error": {
        "code": 0,
        "message": ""
    }
}
```