<?php

class HEX
{
	public $data = [];
	public $start = 0;
	public $end = 0;

	public function dbg($data)
	{
		if (gettype($data) == 'object' || gettype($data) == 'array')
		{
			echo print_r($data, true);
		}
		else
		{
			echo $data;
		}
		echo "\n";
	}

	public function __construct(string $file, int $empty = 255)
	{
		if (!($hex = file_get_contents($file))) die('');

		$this->dbg('HEX readed ' . $file);

		$highAddress = 0;

		$pHEX		= '[0-9a-f]';
		$pLength	= "(?<length>$pHEX{2})";
		$pAddress	= "(?<address>$pHEX{4})";
		$pType		= "(?<type>$pHEX{2})";
		$pData		= "(?<data>$pHEX*)";
		$pCRC		= "(?<crc>$pHEX{2})";
		
		$mi = preg_match_all("/:$pLength$pAddress$pType$pData$pCRC\r?\n/iusU", $hex, $m, PREG_SET_ORDER);

		$this->dbg('HEX lines ' . $mi);
		foreach ($m as $line)
		{
			$type = sscanf($line['type'], '%02x')[0];
			if ($type == 1)
			{
				break;
			}
			if ($type == 0)
			{
				$length		= sscanf($line['length'], '%02x')[0];
				$address	= sscanf($line['address'], '%04x')[0];
				$data		= sscanf($line['data'], str_repeat('%02x', $length));
				$crc		= sscanf($line['crc'], '%02x')[0];

				$crcNew = $length + $type + ($address & 0xFF) + (($address >> 8) & 0xFF);
				foreach($data as $b) $crcNew += $b;
				$crcNew = (($crcNew ^ 0xFF) + 1) & 0xFF;

				if ($crc == $crcNew)
				{
					$i = $highAddress + $address;
					foreach ($data as $b)
					{
						$this->data[$i] = $b;
						$i++;
					}
				}
				else
				{
					$this->dbg('ERR crc ' . $crc . '/' . $crcNew . ' line ' . $line[0]);
					break;
				}
			}
			else if ($type == 4)
			{
				$highAddress = sscanf($line['data'], '%04x')[0] * 0x10000;
			}
			else if ($type == 5)
			{

			}
			else
			{
				$this->dbg('ERR type ' . $type . ' line ' . $line[0]);
			}
		}
		$keys = array_keys($this->data);
		$this->start = min($keys);
		$this->end = max($keys);
		unset($keys);
		if (($this->end - $this->start + 1) != count($this->data))
		{
			for ($i = $this->start; $i <= $this->end; $i++)
			{
				if (!isset($this->data[$i]))
				{
					$this->data[$i] = $empty;
				}
			}
		}
		$this->dbg('HEX binary ' . count($this->data));
		$this->dbg('HEX address ' . sprintf('0x%08x - 0x%08x', $this->start, $this->end));
	}

	public function __toString()
	{
		$s = '';
		/*for ($i = 0; $i < 32768; $i++)
		{
			$s .= chr(255);
		}*/
		for ($i = $this->start; $i <= $this->end; $i++)
		{
			$s .= chr($this->data[$i]);
		}
		return $s;
	}

	public function align(int $align, int $empty = 255)
	{
		$i = ($this->end - $this->start + 1) % $align;
		if ($i > 0)
		{
			$n = $this->end;
			for ($align -= $i; $align > 0; $align--)
			{
				$n++;
				$this->data[$n] = $empty;
			}
			$this->end = $n;
		}
		return count($this->data);
	}
}