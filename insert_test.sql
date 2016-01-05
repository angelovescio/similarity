use similarity;
INSERT INTO `hashes` (`hash`) VALUES ('7833158724726165092')  
	ON DUPLICATE KEY UPDATE id=LAST_INSERT_ID(id), `hash`='7833158724726165092'; 
    INSERT INTO `paths` (`path`,`hashid`,`hashpath`) VALUES ('C:\$WINDOWS.~BT\boot\bootsect.exe',LAST_INSERT_ID(),md5('C:\$WINDOWS.~BT\boot\bootsect.exe'));
INSERT INTO `hashes` (`hash`) VALUES ('7833158724726165092')  
	ON DUPLICATE KEY UPDATE id=LAST_INSERT_ID(id), `hash`='7833158724726165092'; 
    INSERT INTO `paths` (`path`,`hashid`,`hashpath`) VALUES ('C:\$WINDOWS.~BT\boot\en-us\bootsect.exe.mui',LAST_INSERT_ID(),md5('C:\$WINDOWS.~BT\boot\en-us\bootsect.exe.mui'));