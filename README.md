a tool to modify/restore file-type value in file header

```shell
	--encrypt <filename> <value> modify file-type with the given 4-Bytes 
                                     hex value, save the original value to
                                     restore.avi
	--decrypt <filename> restore file-type from restore.avi
	--help show this message
```

## changelog
- v1.1 save the original header value to restore.avi(4B avi_head+ 4B data), this is intented to trick some stupid netdrives who limit the file-type allowed for upload.
- v1.0 save the original header value to restore.bin(4B data)
