CREATE DATABASE if not exists mailbox;

drop table `mailbox`.`mail_box_image`;

CREATE TABLE `mailbox`.`mail_box_image` (
  `ID` VARCHAR(32) NOT NULL,
  `USER_ID` VARCHAR(90) NOT NULL,
  `DEVICE_ID` VARCHAR(90) NOT NULL,
  `DEVICE_STATE` INT NOT NULL,
  `IMAGE_NAME` VARCHAR(36) NOT NULL,
  `TIMESTAMP` TIMESTAMP NOT NULL,
  PRIMARY KEY (`ID`));
