<?php

include 'api/hex.php';
include 'api/hash.php';

$hex = new HEX('Sirena5-BT.hex');
$hex->align(2048);

class PACKET
{
	private $data;
	private static $key = '5C6C6E3D797A7B644E72594C4D295E4352676B215F244F2C324F4D3B6B462833';
	private static $cipher = 'aes-256-cbc';

	public function __construct($address, $hex)
	{
		$this->data[0] = $address & 0xFF;
		$this->data[1] = ($address >> 8) & 0xFF;
		$this->data[2] = ($address >> 16) & 0xFF;
		$this->data[3] = ($address >> 24) & 0xFF;
		for ($i = 0; $i < 2048; $i++)
		{
			$this->data[$i + 4] = $hex->data[$address + $i];
		}
		for ($i = 0; $i < 10; $i++)
		{
			$this->data[$i + 2052] = 0;
		}
		$crc = HASH::crc16arr($this->data, 0, 2062);
		$this->data[2062] = $crc & 0xFF;
		$this->data[2063] = ($crc >> 8) & 0xFF;
	}

	private static function arr2str(array $arr)
	{
		$s = '';
		foreach($arr as $v) $s .= chr($v);
		return $s;
	}

	public function __toString()
	{
		$cmd = 'openssl enc -' . self::$cipher;
		$cmd .= ' -K "' . self::$key . '"';
		$cmd .= ' -iv "' . substr(self::$key, 0, 32) . '"';
		$cmd .= ' -nosalt -nopad -a -A';
		
		$p = proc_open
			(
				$cmd,
				[
					0 => ['pipe', 'r'],
					1 => ['pipe', 'w'],
					2 => ['file', '/dev/null', 'a'],
				],
				$pipe
			);

		fwrite($pipe[0], self::arr2str($this->data));
		fclose($pipe[0]);

		$e = stream_get_contents($pipe[1]);
		fclose($pipe[1]);

		proc_close($p);

		return $e;
	}
}

$d = '';
for ($i = $hex->start, $n = 0; $i <= $hex->end; $i += 2048, $n++)
{
	$d .= (new PACKET($i, $hex)) . "\r\n";
}

file_put_contents('Sirena5-BT.fw', $d);
