<?

function microtime_float()
{
    list($usec, $sec) = explode(" ", microtime());
    return ((float)$usec + (float)$sec);
}

$time_start = microtime_float();

$fp = @fsockopen("221.139.48.76",9099, $errno, $errstr,3);
if ($fp)
{
 echo "소켓 연결 성공";
}
else
{
 echo "데몬이 죽어 있음.$errstr ($errno)";
 exit();
}
echo "연결 : ".fgets($fp, 1024);

for ($i = 0 ; $i < 1; $i ++ ) {
$buf = "CM_BEGIN\n
인천시 외부전문가 '시민식품감사인제' 운영. 인천시는 식품제, 가공업자의 위생관리 능력을 향상시키기 위해 
'시민식품감사인'제도를 도입, 운영할 방침이라고 23일 밝혔다. 이는 식품제조 및 가공업자가 자율적인 위생점검을 위해 시민식품감사인을 지정해 신청할 경우, 
시에서 일정자격을 갖춘 사람 2명 이상을 선정해 통보하고, 사업주는 이들 가운데 1명을 식품감사인으로 위촉 하는 제도이다. 시민식품감사인을 위촉한 업소는 분기별로 
1회씩 시민식품감사인을 >통해 식품 등의 위생 취급기준 이행 등의 위생점검을 받게 되며, 시민식품감사인 위촉기간 중 공무원의 출입과 검사를 면제받을 수 있다. 시는 이에 따라 인천
시내 식품제조,가공업소 중 시민식품감사인 위촉을 희망하는 업소의 신청을 받아 오는 4월 예비 시민식품감사인 중 2명 이상을 지정, 통보할 방침이다. 
시 관계자는 \"외부 전문가인 시민식품감사인을 통해 위생상태를 정기적으로 점검받도록 해 위생관리의 공정성과 투명성을 높일 방침\"이라며 \"이 제도 운영으로 >부족한 식품위생감시 인력을 확보할 수 있다\"라고 말했다.
CM_END\n";
fputs($fp, $buf);
echo "[결과]\n";
$buf = fgets($fp, 102400);
$tmp_json = json_decode($buf, TRUE);

//for ($i = 0 ; $i < sizeof($tmp_json[category_ad]) ; $i ++ ) {
//	$tmp_json[category_ad][$i][cat_id] = $i;
//}

print_r($tmp_json);
//$en_json = json_encode($tmp_json);
echo "$buf\nlength:".strlen($buf)."\n";

}

fputs($fp, "quit\n");
while (!feof($fp))
{
 //서버에서 출력한 스트링을 다가지고 온다.
 echo fgets($fp, 1024);
 echo "\n";
}
fclose ($fp);

$time_end = microtime_float();
$time = $time_end - $time_start;

echo "Did nothing in $time seconds\n";

?>
