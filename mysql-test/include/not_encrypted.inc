if (`select @@innodb_encryption = 1`)
{
  skip only without encryption;
}
