use std::borrow::Cow;
use std::collections::{HashMap, VecDeque};
use std::path::PathBuf;
use std::{panic::PanicHookInfo, path::Path};

use error::PakError;
use kinded::{Kind, Kinded};
use winnow::binary::{be_u32, le_u32, u8};
use winnow::combinator::alt;
use winnow::error::ContextError;
use winnow::token::take;
use winnow::{Parser, Result as WResult};

mod error;

#[derive(Debug)]
pub struct PakFile {}

#[derive(Debug)]
enum PakType {
    PAK1,
}

#[derive(Debug)]
struct FileEntry<'input> {
    kind: FileEntryKind,
    expected_children_count: usize,
    name: Cow<'input, str>,
    children: Vec<FileEntry<'input>>,
    offset: u32,
    compressed_len: u32,
    decompressed_len: u32,
    compression_type: u32,
    unknown: u32,
    timestamp: u32,
}

#[derive(Debug, Kinded)]
enum Chunk<'input> {
    Pac1(u32),
    Form {
        file_size: u32,
        pak_file_type: PakType,
    },
    Head {
        version: u32,
        header_data: &'input [u8],
    },
    Data {
        data: &'input [u8],
    },
    File {
        entries: Vec<FileEntry<'input>>,
    },
    Unknown(u32),
}

impl PakFile {
    pub fn parse(path: &PathBuf) -> Result<PakFile, PakError> {
        let file = std::fs::File::open(path)?;
        let file = unsafe { memmap2::Mmap::map(&file)? };

        parse_pak(&file[..])
    }
}

fn parse_pak(mut input: &[u8]) -> Result<PakFile, PakError> {
    let result = PakFile {};

    loop {
        match parse_chunk(&mut input) {
            Ok(chunk) => {
                if let Chunk::Data { data } = chunk {
                    println!("Data chunk with len: {:#02X}", data.len());
                } else {
                    println!("{:#?}", chunk);
                }
            }
            Err(e) => {
                println!("{:?}", e);
                break;
            }
        }
    }

    Ok(result)
}

fn parse_form_chunk<'input>(input: &mut &'input [u8]) -> WResult<Chunk<'input>> {
    let file_size = be_u32(input)?;
    let pak_type_bytes: [u8; 4] = take(4usize)
        .parse_next(input)?
        .try_into()
        .expect("winnow should have returned a 4-byte buffer");
    let pak_file_type = match &pak_type_bytes {
        b"PAC1" => PakType::PAK1,
        unk => {
            panic!("unknown pak type: {:?}", unk);
        }
    };
    Ok(Chunk::Form {
        file_size,
        pak_file_type,
    })
}

fn parse_head_chunk<'input>(input: &mut &'input [u8]) -> WResult<Chunk<'input>> {
    let header_len = be_u32(input)?;
    assert_eq!(header_len, 0x1c);

    let mut header_data = take(header_len).parse_next(input)?;
    let version = le_u32(&mut header_data)?;

    let chunk = Chunk::Head {
        version,
        header_data,
    };

    Ok(chunk)
}

fn parse_data_chunk<'input>(input: &mut &'input [u8]) -> WResult<Chunk<'input>> {
    let data_len = be_u32(input)?;

    let data = take(data_len).parse_next(input)?;

    let chunk = Chunk::Data { data };

    Ok(chunk)
}

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord)]
enum FileEntryKind {
    Folder,
    File,
}

impl TryFrom<u8> for FileEntryKind {
    type Error = ();

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        let result = match value {
            0 => Self::Folder,
            1 => Self::File,
            _ => {
                panic!("unknown file entry kind: {:#X}", value);
            }
        };

        Ok(result)
    }
}

fn parse_file_entry<'input>(input: &mut &'input [u8]) -> WResult<FileEntry<'input>> {
    let entry_kind: FileEntryKind = u8(input)?.try_into().expect("???");
    let name_len = u8(input)?;
    let name_bytes = take(name_len).parse_next(input)?;
    // TODO: use proper error type
    let name = str::from_utf8(name_bytes).expect("invalid utf8 filename");

    match entry_kind {
        FileEntryKind::Folder => {
            let children_count = le_u32(input)?;
            if name_len == 0 {
                // Special case for root directory
                return Ok(FileEntry {
                    kind: FileEntryKind::Folder,
                    name: Cow::Owned("Root".to_string()),
                    expected_children_count: children_count as usize,
                    children: vec![],
                    offset: 0,
                    compressed_len: 0,
                    decompressed_len: 0,
                    compression_type: 0,
                    unknown: 0,
                    timestamp: 0,
                });
            }

            return Ok(FileEntry {
                kind: entry_kind,
                name: Cow::Borrowed(name),
                expected_children_count: children_count as usize,
                children: vec![],
                offset: 0,
                compressed_len: 0,
                decompressed_len: 0,
                compression_type: 0,
                unknown: 0,
                timestamp: 0,
            });
        }
        FileEntryKind::File => {
            let offset = le_u32(input)?;
            let compressed_len = le_u32(input)?;
            let decompressed_len = le_u32(input)?;
            let unknown = le_u32(input)?;
            let compression_type = le_u32(input)?;
            let timestamp = le_u32(input)?;

            if compressed_len != decompressed_len {
                println!("compression_type: {unknown}, filename: {name}, unk: {compression_type}");
            }
            assert_eq!(unknown, 0);

            return Ok(FileEntry {
                kind: entry_kind,
                name: Cow::Borrowed(name),
                expected_children_count: 0,
                children: vec![],
                offset,
                compressed_len,
                decompressed_len,
                compression_type,
                unknown,
                timestamp,
            });
        }
    }
}

fn parse_file_chunk<'input>(input: &mut &'input [u8]) -> WResult<Chunk<'input>> {
    let chunk_len = be_u32(input)?;

    dbg!(chunk_len);

    let mut chunk_data = take(chunk_len).parse_next(input)?;
    dbg!(chunk_data.len());

    let mut entries = Vec::new();

    while !chunk_data.is_empty() {
        let entry = parse_file_entry(&mut chunk_data)?;
        entries.push(entry);
    }

    let chunk = Chunk::File { entries };

    Ok(chunk)
}

fn parse_chunk<'input>(input: &mut &'input [u8]) -> WResult<Chunk<'input>> {
    let fourcc: [u8; 4] = take(4usize)
        .parse_next(input)?
        .try_into()
        .expect("winnow should have returned a 4-byte buffer");

    match &fourcc {
        b"FORM" => parse_form_chunk(input),
        b"HEAD" => parse_head_chunk(input),
        b"DATA" => parse_data_chunk(input),
        b"FILE" => parse_file_chunk(input),
        unk => {
            panic!("Unknown chunk: {:?}", unk);
        }
    }
}
